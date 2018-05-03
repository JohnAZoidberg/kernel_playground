.PHONY: clean help

obj-m += ramdisk.o
obj-m += funmod_base.o
obj-m += funmod_extension.o
funmod-objs := funmod.o

all: module

SRC_DIR := $(shell pwd)

# get the correct $dev variable by launching the following nix-shell
# nix-shell '<nixpkgs>' -A linuxPackages_4_14.kernel
module:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) modules

help:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) help

clean:
	$(MAKE) -C $(dev)/lib/modules/*/build M=$(SRC_DIR) clean
