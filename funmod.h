int init_module(void);
void cleanup_module(void);
//static int device_open(struct inode *, struct file *);
//static int device_release(struct inode *, struct file *);
//static ssize_t device_read(struct file *, char *, size_t, loff_t *);
//static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define BUF_LEN 80              /* Max length of the message from the device */

#ifndef QUERY_IOCTL_H
#define QUERY_IOCTL_H

typedef struct
{
	int foo;
	int foobar;
	int baz;
} query_arg_t;

#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, query_arg_t *)

#endif
