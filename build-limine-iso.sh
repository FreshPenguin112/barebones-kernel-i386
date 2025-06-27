#!/bin/bash
set -e

# Paths (edit as needed)
LIMINE_DIR=../limine # Path to Limine build directory
KERNEL=mykernel.elf
ISO=kernel.iso
ISO_ROOT=iso_root

# 1. Build kernel (x86_64)
gcc -std=gnu99 -ffreestanding -m64 -g -c *.s *.c
gcc -ffreestanding -nostdlib -m64 -g -T linker.ld *.o -o $KERNEL -lgcc

# 2. Build userland programs (x86_64)
for src in userland/*.c; do
    fname=$(basename "$src" .c)
    if [ "$(echo "$fname" | cut -c1)" != "_" ]; then
        gcc -ffreestanding -nostdlib -m64 -g -Ttext=0x800000 string_utils.c "$src" -o "initfs/$fname.elf"
    fi
done

# 3. Prepare ISO root
echo "Preparing ISO root..."
rm -rf $ISO_ROOT
mkdir -p $ISO_ROOT/boot/limine
cp $KERNEL $ISO_ROOT/boot/limine/
cp limine.conf $ISO_ROOT/boot/limine/
cp $LIMINE_DIR/limine-bios.sys $ISO_ROOT/boot/limine/
cp $LIMINE_DIR/limine-bios-cd.bin $ISO_ROOT/boot/limine/
cp $LIMINE_DIR/limine-uefi-cd.bin $ISO_ROOT/boot/limine/
cp $LIMINE_DIR/BOOTX64.EFI $ISO_ROOT/boot/limine/
cp $LIMINE_DIR/BOOTIA32.EFI $ISO_ROOT/boot/limine/

# Optionally copy userland binaries to ISO (if needed)
cp -r initfs $ISO_ROOT/

# 4. Create ISO
xorriso -as mkisofs -R -J -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --efi-boot boot/limine/limine-uefi-cd.bin \
    --efi-boot-part --efi-boot-image --protective-msdos-label \
    $ISO_ROOT -o $ISO

# 5. Install Limine to ISO
$LIMINE_DIR/limine bios-install $ISO

echo "Limine x86_64 ISO ready: $ISO"
