#!/bin/bash
set -e

# Paths (edit as needed)
LIMINE_DIR=../limine # Path to Limine build directory
KERNEL=mykernel.elf
IMG=hdd.img
MNT=mnt

# 1. Create 64MB blank image
if [ ! -f "$IMG" ]; then
    dd if=/dev/zero of=$IMG bs=1M count=64
    parted $IMG --script mklabel msdos
    parted $IMG --script mkpart primary fat32 1MiB 100%
    parted $IMG --script set 1 boot on
    sync
fi
sleep 1
# 2. Set up loop device
LOOPDEV=$(sudo losetup --find --show --partscan $IMG)
mkfs.fat -F 32 ${LOOPDEV}p1

# 3. Mount
mkdir -p $MNT
mount ${LOOPDEV}p1 $MNT

# 4. Copy kernel, Limine files, and config (new Limine v9.x+ layout)
mkdir -p $MNT/boot/limine
cp $KERNEL $MNT/boot/limine/
cp limine.conf $MNT/boot/limine/
cp $LIMINE_DIR/limine-bios.sys $MNT/boot/limine/

# 5. Install Limine (BIOS)
$LIMINE_DIR/limine bios-install $IMG

ls $MNT/boot/limine/
ls $MNT/boot/
cat $MNT/boot/limine/limine.conf
sync
# 6. Unmount and detach
cp minimal_bios.elf $MNT/boot/limine/
umount $MNT
losetup -d $LOOPDEV

# Optionally build minimal BIOS test kernel
# gcc -ffreestanding -m32 -nostdlib -fno-pic -fno-pie -Wl,--entry=start -o minimal_bios.elf minimal_bios.c start.s limine.h



echo "Limine disk image ready: $IMG"
