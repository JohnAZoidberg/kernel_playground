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
// 204800 sectors are 100MiB of memory
module_param(sector_count, ulong, S_IRUGO);
MODULE_PARM_DESC(sector_count, "Count of sectors the ramdisk should contain. One sector is 512 bytes.");

#define SECTOR_SIZE 512
#define DEV_COUNT 3

static DEFINE_MUTEX(ramdisk_devices_mutex);

static int major = 0;
static struct gendisk *disks[DEV_COUNT];
static DEFINE_SPINLOCK(spinlock);
static char *memory[DEV_COUNT];
static struct block_device_operations bdops = {
	.owner = THIS_MODULE,
};


// This function is responsible for working on the kernel requests
static void ramdisk_request(struct request_queue *q)
{
	struct request *req;

	struct bio_vec bvec;
	struct req_iterator iter;
	char *memoryAddr, *bioAddr;

	req = blk_fetch_request(q);

	while (req) {
		//printk(KERN_INFO "Request for minor: %d", req->rq_disk->first_minor);
		//printk(KERN_INFO "Request for minor: %s", req->rq_disk->disk_name);
		// Iterate through each page of the BIO structure
		rq_for_each_segment(bvec, req, iter) {
			// BIO gives you a whole page in which there is data that should be written from or to.
			// Therefore you first have to calculate the memory address out of the page address.
			bioAddr = page_address(bvec.bv_page) + bvec.bv_offset;

			if (iter.bio->bi_iter.bi_sector >= sector_count) {
				printk(KERN_ERR "Tried to read or write OUT OF BOUNDS!");
				// If this returns true, there is still work left to do with this request,
				// so we should not get another one.
				if (!__blk_end_request_cur(req, BLK_STS_IOERR))
					req = blk_fetch_request(q);
				continue;
			}
			//Block devices access sectors, we have to calculate the memory address out of it.
			memoryAddr = memory[req->rq_disk->first_minor] + (SECTOR_SIZE * iter.bio->bi_iter.bi_sector); // TODO FIXME!!!

			if (bio_data_dir(iter.bio) == READ)
				memcpy(bioAddr, memoryAddr, bvec.bv_len);
			else // WRITE
				memcpy(memoryAddr, bioAddr, bvec.bv_len);
		}

		// If this returns true, there is still work left to do with this request,
		// so we should not get another one.
		if (!__blk_end_request_cur(req, BLK_STS_OK))
				req = blk_fetch_request(q);
	}
}

static struct kobject *ramdisk_probe(dev_t dev, int *part, void *data)
{
	printk(KERN_INFO "Part is: %d.", *part);
	return get_disk_and_module(disks[0]);
}


// Initialization
static int __init ramdisk_init(void)
{
	int disk, err;

	// Register the block device
	major = register_blkdev(0, "ramdisk");
	if (major < 1) { // TODO check what values the return value can have
		printk(KERN_ERR "Failed to acquire device file!");
		return major;
	}
	printk(KERN_INFO "Registered blkdev");

	blk_register_region(MKDEV(major, 0), 8, THIS_MODULE, ramdisk_probe, NULL, NULL);
	printk(KERN_INFO "Registered registered region");

	for (disk = 0; disk < DEV_COUNT; disk++) {
		printk(KERN_INFO "Trying to create ramdisk%d.", disk);
		// Allocate the disk space
		memory[disk] = vmalloc(sector_count * SECTOR_SIZE);
		if (!memory[disk]) {
			printk(KERN_ERR "Failed to acquire a big chunk of memory for the disk space!");
			err = -ENOMEM;
			goto out_free_cleanup_queue;
		}
		printk(KERN_INFO "Allocated memory for disk");
		// Allocate a gendisk struct and initialize with the values of our ramdisk
		disks[disk] = alloc_disk(1); // TODO do we need 16? 1 should be enough. What does it mean?
		if (!disks[disk]) {
			printk(KERN_ERR "Failed to generate gendisk struct!");
			err = -ENOMEM;
			goto out_free_cleanup_queue;
		}
		printk(KERN_INFO "Allocated disk");

		disks[disk]->major = major;
		disks[disk]->first_minor = disk;
		set_capacity(disks[disk], sector_count);
		sprintf(disks[disk]->disk_name, "ramdisk%d", disk);
		disks[disk]->fops = &bdops;
		// Generate a request_queue, it is needed for getting access requests from the kernel
		disks[disk]->queue = blk_init_queue(&ramdisk_request, &spinlock);
		if (!disks[disk]->queue) {
			printk(KERN_ERR "Failed to generate request_queue struct!");
			err = -ENOMEM;
			goto out_free_cleanup_queue;
		}

		// HELL YEAH, IT WORKED!
		add_disk(disks[disk]);
		printk(KERN_INFO "Successfully created/added ramdisk%d.", disk);
	}
	printk(KERN_INFO "Successfully created all ramdisks.");
	return 0;
out_free_cleanup_queue:
	printk(KERN_INFO "Ramdisk: out_free_cleanup_queue (vfree)");
	for (disk = 0; disk < DEV_COUNT; disk++) {
		if (memory[disk])
			vfree(memory[disk]);
		if (disks[disk]->queue)
			blk_cleanup_queue(disks[disk]->queue);
	}
	printk(KERN_INFO "Ramdisk: out_free_cleanup_queue (blk_cleanup_queue)");
	printk(KERN_INFO "Ramdisk: out_unregister");
	blk_unregister_region(MKDEV(major, 0), 1UL << MINORBITS);
	unregister_blkdev(major, "ramdisk");
	return err;
}


// Cleanup
static void __exit ramdisk_exit(void)
{
	int disk;
	// Basically delete everything we created in the init function
	printk(KERN_INFO "Destroying ramdisk...");
	for (disk = 0; disk < DEV_COUNT; disk++) {
		del_gendisk(disks[disk]);
		put_disk(disks[disk]);
		if (memory[disk])
			vfree(memory[disk]);
		if (disks[disk]->queue)
			blk_cleanup_queue(disks[disk]->queue);
		printk(KERN_INFO "Successfully destroyed ramdisk%d.", disk);
	}
	blk_unregister_region(MKDEV(major, 0), 1UL << MINORBITS);
	unregister_blkdev(major, "ramdisk");
	printk(KERN_INFO "Every ramdisk successfully destroyed...");
}


module_init(ramdisk_init);
module_exit(ramdisk_exit);
