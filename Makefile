.PHONY: clean help

obj-m += ramdisk.o
obj-m += funmod.o
funmod-objs := funmod_base.o funmod_extension.o

all: module userprog

SRC_DIR := $(shell pwd)

# get the correct $dev variable by launching the nix-shell
# but first populate the $dev location by building it
# nix-build $nixchannels -A linuxPackages_4_14.kernel.dev
# nix-shell '<nixpkgs>' -A linuxPackages_4_14.kernel
module:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) modules
indie:
	$(MAKE) -C /home/zoid/media/clone/linux/ M=$(SRC_DIR) modules

help:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) help

clean:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) clean
	rm ioctl_user

userprog:
	gcc -o ioctl_user ioctl_user.c
