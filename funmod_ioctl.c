#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/version.h>

#include "funmod.h"

#define FIRST_MINOR 0
#define MINOR_CNT 1

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int foo = 3;
static int foobar = 1;
static int baz = 4;

// we only allow ioctl no open and close
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

int ioctl_init(void)
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
	return 0;
}
void ioctl_exit(void)
{
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
}
