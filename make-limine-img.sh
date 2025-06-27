#!/bin/bash
set -e

IMG=mykernel.img
KERNEL=mykernel.elf
LIMINE=../limine
ISO=isodir

# Clean up
umount $ISO 2>/dev/null || true
rm -f $IMG
rm -rf $ISO

# Create empty 64MB image
dd if=/dev/zero of=$IMG bs=1M count=64

# Partition and format image
sgdisk -n 1:2048: -t 1:ef00 $IMG
LOOPDEV=$(losetup --find --show $IMG)
PARTDEV=${LOOPDEV}p1
partprobe $LOOPDEV
mkfs.vfat $PARTDEV

# Mount and copy files
mkdir -p $ISO
mount $PARTDEV $ISO
mkdir -p $ISO/boot/limine
cp $KERNEL $ISO/boot/
cp $LIMINE/limine-bios.sys $LIMINE/limine-bios-cd.bin $LIMINE/limine-uefi-cd.bin $LIMINE/BOOTX64.EFI $LIMINE/BOOTIA32.EFI limine/limine.conf $ISO/boot/limine/
mkdir -p $ISO/EFI/BOOT
cp $LIMINE/BOOTX64.EFI $ISO/EFI/BOOT/
cp $LIMINE/BOOTIA32.EFI $ISO/EFI/BOOT/

# Unmount and detach
sync
umount $ISO
losetup -d $LOOPDEV

# Install Limine
$LIMINE/limine bios-install $IMG

echo "Limine .img ready: $IMG"
