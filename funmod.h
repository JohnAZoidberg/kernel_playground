int init_module(void);
void cleanup_module(void);

int ioctl_init(void);
void ioctl_exit(void);

int sysfs_init(void);
void sysfs_exit(void);

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
