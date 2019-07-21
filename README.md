# Kernel playground
Playground for practicing Linux kernel programming

## "Funmod" driver
Consists of files: `funmod_base.c` `funmod.h`, `funmod_ioctl.c`, `ioctl_user.c`, `funmod_sysfs.c`.

Creates both a device which reacts to ioctl and a sysfs file that can store an integer.

### ioctl 
The kernel module creates a device at `/dev/query`. It contains a struct

```c
typedef struct
{
	int foo;
	int foobar;
	int baz;
} query_arg_t;
```

that can be queried or modified using ioctls.

| Description                                 | Macro                 | Definition                    |
|---------------------------------------------|-----------------------|-------------------------------|
| Get values                                  | `QUERY_GET_VARIABLES` | `_IOR('q', 1, query_arg_t *)` |
| Defaults values to `foo=3, foobar=1, baz=4` | `QUERY_CLR_VARIABLES` | `_IO('q', 2)`                 |
| Set values to the same as the struct passed | `QUERY_SET_VARIABLES` | `_IOW('q', 3, query_arg_t *)` |


### Sysfs file
Kernel module that creates a file in sysfs at `/sys/funfile/number`.

It's possible to write an integer to it and get it back out (default 42):
```
$ cat /sys/funfile/number
42
$ echo 123 > /sys/funfile/number
$ cat /sys/funfile/number
123
```

## Ramdisk kernel driver (`ramdisk.c`)
- Simple RAM backed block device that has been successfully formatted.
- Can be configured to create more than one device upon loading.
  Dynamic creation and deletion of devices is not yet possible. Seems to leak some memory.

## License
`ramdisk.c` GPLv3+, everything else GPLv2.
