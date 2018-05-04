/*
 * funmod_base.c
 *
 *  Created on: 02.05.2017
 *	  Author: Daniel Schaefer <git@danielschaefer.me>
 *
 *  Module for playing around with linux kernel modules.
 *  Copyright (C) 2018 Daniel Schaefer <git@danielschaefer.me>
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
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/printk.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/version.h>

#include <linux/kobject.h>
#include <linux/sysfs.h>

#include "funmod.h"

#define FIRST_MINOR 0
#define MINOR_CNT 1


// ioctl stuff
static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int foo = 3;
static int foobar = 1;
static int baz = 4;

// we only allow ioctl
static int fun_ioctl_open(struct inode *i, struct file *f)
{
	return 0;
}

static int fun_ioctl_close(struct inode *i, struct file *f)
{
	return 0;
}

static long fun_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	query_arg_t q;
	switch (cmd)
	{
	case QUERY_GET_VARIABLES:
		q.foo = foo;
		q.foobar = foobar;
		q.baz = baz;
		if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
			return -EACCES;
		break;
	case QUERY_CLR_VARIABLES:
		foo = 3;
		foobar = 1;
		baz = 4;
		break;
	case QUERY_SET_VARIABLES:
		if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
			return -EACCES;
		foo = q.foo;
		foobar = q.foobar;
		baz = q.baz;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct file_operations query_fops =
{
	.owner = THIS_MODULE,
	.open = fun_ioctl_open,
	.release = fun_ioctl_close,
	.unlocked_ioctl = fun_ioctl
};

MODULE_AUTHOR("Daniel Schaefer <git@danielschaefer.me>");
MODULE_DESCRIPTION("Sample driver with no purpose");
MODULE_LICENSE("GPL");

static int number = 42;
module_param(number, int, 0644);
MODULE_PARM_DESC(number, "Some number");

static struct kobject *foo_kobject;
static int value;

static ssize_t foo_read(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	return sprintf(buf, "%d\n", value);
}

static ssize_t foo_write(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &value);
	return count;
}

static struct kobj_attribute fun_attribute =
	__ATTR(value, 0660, foo_read, foo_write);

static int __init fun_init(void)
{
	int err;
	struct device *dev_ret;
	if ((err = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT,
				       "query_ioctl")) < 0)
		return err;
	cdev_init(&c_dev, &query_fops);
	if ((err = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
		return err;
	if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(cl);
	}
	if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "query")))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}

	foo_kobject = kobject_create_and_add("funfile", foo_kobject);
	if (!foo_kobject)
		return -ENOMEM;

	if ((err = sysfs_create_file(foo_kobject, &fun_attribute.attr)))
		pr_debug("failed to crate the file at /sys/funfile/value");
	pr_debug("Module initialized successfully \n");
	return err;
}

static void __exit fun_exit(void)
{
	// destroy ioctl stuff
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);

	kobject_put(foo_kobject);
	printk(KERN_ALERT "Today's number was %d", value);
	printk(KERN_ALERT "Unloading fun driver");
}

module_init(fun_init);
module_exit(fun_exit);
