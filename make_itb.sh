#!/bin/sh
rm -f vmlinux.bin*
ppc-linux-objcopy -O binary vmlinux vmlinux.bin
gzip -c vmlinux.bin > vmlinux.bin.gz
lzma -z vmlinux.bin
mkimage -f `dirname $0`/$1.its $1.itb
