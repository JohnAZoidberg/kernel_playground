#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include <linux/kobject.h>
#include <linux/sysfs.h>

#include "funmod.h"

static int number = 42;
module_param(number, int, 0644);
MODULE_PARM_DESC(number, "Some number");

static struct kobject *foo_kobject;

static ssize_t foo_read(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	return sprintf(buf, "%d\n", number);
}

static ssize_t foo_write(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &number);
	return count;
}

static struct kobj_attribute fun_attribute =
	__ATTR(number, 0660, foo_read, foo_write);

int sysfs_init(void)
{
	int err = 0;
	foo_kobject = kobject_create_and_add("funfile", foo_kobject);
	if (!foo_kobject)
		return -ENOMEM;

	if ((err = sysfs_create_file(foo_kobject, &fun_attribute.attr)))
		pr_debug("failed to crate the file at /sys/funfile/number");
	return err;
}

void sysfs_exit(void)
{
	kobject_put(foo_kobject);
	printk(KERN_ALERT "Today's number was %d", number);
}
