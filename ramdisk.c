/*
 * ramdisk.c
 *
 *  Created on: 08.12.2017
 *	  Author: Daniel Rutz <info@danielrutz.com>
 *
 *  A very simple ramdisk block driver for the Linux Operating System.
 *  Copyright (C) 2017 Daniel Rutz <info@danielrutz.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkdev.h>


MODULE_AUTHOR("Daniel Rutz <info@danielrutz.com>");
MODULE_DESCRIPTION("A simple ramdisk driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("ramdisk");


static unsigned long sector_count = 204800;
//204800 sectors are 100MiB of memory
module_param(sector_count, ulong, S_IRUGO);
MODULE_PARM_DESC(sector_count, "Count of sectors the ramdisk should contain. One sector is 512 bytes.");


#define SECTOR_SIZE 512


static int major = 0;
static struct gendisk* disk = NULL;
static struct request_queue* queue = NULL;
static DEFINE_SPINLOCK(spinlock);
static char* memory = NULL;
static struct block_device_operations bdops = {
	.owner = THIS_MODULE,
};


//This function is responsible for working on the kernel requests
static void ramdisk_request(struct request_queue* q)
{
	struct request* req;

	struct bio_vec bvec;
	struct req_iterator iter;
	char* memoryAddr, *bioAddr;

	req = blk_fetch_request(q);

	while (req) {
		//Iterate through each page of the BIO structure
		rq_for_each_segment(bvec, req, iter) {
			//BIO gives you a whole page in which there is data that should be written from or to. Therefore you first have to calculate the memory address out of the page address.
			bioAddr = page_address(bvec.bv_page) + bvec.bv_offset;

			if (iter.bio->bi_iter.bi_sector >= sector_count) {
				printk(KERN_ERR "Tried to read or write OUT OF BOUNDS!");
				if (!__blk_end_request_cur(req, BLK_STS_IOERR)) { //If this returns true, there is still work left to do with this request, so we should not get another one.
					req = blk_fetch_request(q);
				}
				continue;
			}
			memoryAddr = memory + (SECTOR_SIZE * iter.bio->bi_iter.bi_sector); //Block devices access sectors, we have to calculate the memory address out of it.

			if (bio_data_dir(iter.bio) == READ) {
				memcpy(bioAddr, memoryAddr, bvec.bv_len);
			}
			else { //WRITE
				memcpy(memoryAddr, bioAddr, bvec.bv_len);
			}
		}

		if (!__blk_end_request_cur(req, BLK_STS_OK)) { //If this returns true, there is still work left to do with this request, so we should not get another one.
				req = blk_fetch_request(q);
		}
	}
}


//Initialization
static int __init ramdisk_init(void)
{
	//Allocate the disk space
	memory = vmalloc(sector_count * SECTOR_SIZE);
	if (!memory) {
		printk(KERN_ERR "Failed to acquire a big chunk of memory for the disk space!");
		return -ENOMEM;
	}

	//Register the block device
		major = register_blkdev(0, "ramdisk");
	if (major < 1) {
		printk(KERN_ERR "Failed to acquire device file!");
		vfree(memory);
		return -EIO;
	}

	//Generate a request_queue, it is needed for getting access requests from the kernel
	queue = blk_init_queue(&ramdisk_request, &spinlock);
	if (!queue) {
		printk(KERN_ERR "Failed to generate request_queue struct!");
		unregister_blkdev(major, "ramdisk");
		vfree(memory);
		return -ENOMEM;
	}

	//Allocate a gendisk struct and initialize with the values of our ramdisk
	disk = alloc_disk(16);
	if (!disk) {
		printk(KERN_ERR "Failed to generate gendisk struct!");
		blk_cleanup_queue(queue);
		unregister_blkdev(major, "ramdisk");
		vfree(memory);
		return -ENOMEM;
	}

	disk->major = major;
	disk->first_minor = 0;
	set_capacity(disk, sector_count);
	sprintf(disk->disk_name, "ramdisk");
	disk->fops = &bdops;
	disk->queue = queue;

	//HELL YEAH, IT WORKED!
	printk(KERN_INFO "Successfully created ramdisk.");
	add_disk(disk);
	return 0;
}


//Cleanup
static void __exit ramdisk_exit(void)
{
	//Basically delete everything we created in the init function
	printk(KERN_INFO "Destroying ramdisk...");
	del_gendisk(disk);
	put_disk(disk);
	blk_cleanup_queue(queue);
	unregister_blkdev(major, "ramdisk");
	vfree(memory);
}


module_init(ramdisk_init);
module_exit(ramdisk_exit);
