#!/bin/sh

K=/Volumes/btc-0.11/build/linux-2.6.27

set -e -u -x

rm -f code code-gen init $K/usr/initramfs_data.cpio{,.gz}

gcc code-gen.c -o code-gen
./code-gen > code

m68k-linux-gnu-gcc -static -o init init.c 
m68k-linux-gnu-strip init

$K/usr/gen_init_cpio gen_init_cpio.list > $K/usr/initramfs_data.cpio
gzip -9 $K/usr/initramfs_data.cpio
cmp dot-config $K/.config
(cd $K && make ARCH=${TARGET_CPU} CROSS_COMPILE=${TARGET}-)

echo all done
