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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#include "funmod.h"

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
	int error = 0;
	pr_debug("Module initialized successfully \n");

	foo_kobject = kobject_create_and_add("funfile", foo_kobject);
	if (!foo_kobject)
		return -ENOMEM;

	error = sysfs_create_file(foo_kobject, &fun_attribute.attr);
	if (error)
		pr_debug("failed to crate the file at /sys/funfile/value");
	return error;
}

static void __exit fun_exit(void)
{
	kobject_put(foo_kobject);
	printk(KERN_ALERT "Today's number was %d", value);
	printk(KERN_ALERT "Unloading fun driver");
}

module_init(fun_init);
module_exit(fun_exit);
