#!/bin/bash
set -e

# Paths
LIMINE_DIR="../limine"
ISO_DIR="isodir_x86_64"
KERNEL_ELF="mykernel_x86_64.elf"
ISO_NAME="mykernel_x86_64.iso"

# Clean up
rm -rf "$ISO_DIR" "$KERNEL_ELF" "$ISO_NAME"
rm -f *.o userland/*.o initfs/*.o initfs/*.elf initfs.tar initramfs.c
mkdir -p "$ISO_DIR"

# Build userland programs (64-bit)
USERLAND_DIR="userland"
INITFS_DIR="initfs"
rm -rf "$INITFS_DIR"
mkdir -p "$INITFS_DIR"
for src in $USERLAND_DIR/*.c; do
    fname=$(basename "$src" .c)
    gcc -mcmodel=kernel -ffreestanding -nostdlib -g -Ttext=0x800000 string_utils.c "$src" -o "$INITFS_DIR/$fname.elf"
done

# Copy userland ELF files to ISO
mkdir -p "$ISO_DIR/initfs"
cp $INITFS_DIR/*.elf "$ISO_DIR/initfs/"

# Create initfs tarball for kernel
cd "$INITFS_DIR"
tar cf ../initfs.tar .
cd ..

# Convert initfs.tar to C array using xxd for kernel inclusion
xxd -i initfs.tar > initramfs.c

# Build kernel (64-bit)
gcc -mcmodel=kernel -std=gnu99 -ffreestanding -g -c *.c *.s
gcc -mcmodel=kernel -ffreestanding -nostdlib -g -T linker-x86_64.ld *.o -o "$KERNEL_ELF" -lgcc

# Prepare ISO root
cat > "$ISO_DIR/limine.conf" <<EOF
timeout: 0

/Limine Template
    protocol: limine

    path: boot():/kernel.elf
EOF
cp "$KERNEL_ELF" "$ISO_DIR/kernel.elf"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_DIR/"
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_DIR/"
cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_DIR/"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_DIR/"
mkdir -p "$ISO_DIR/EFI/BOOT"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_DIR/EFI/BOOT/BOOTX64.EFI"

# Create ISO
xorriso -as mkisofs -b limine-bios-cd.bin \
  -no-emul-boot -boot-load-size 4 -boot-info-table \
  --efi-boot limine-uefi-cd.bin \
  --efi-boot-part --efi-boot-image --protective-msdos-label \
  -o "$ISO_NAME" "$ISO_DIR"

# Deploy Limine
"$LIMINE_DIR/limine" bios-install "$ISO_NAME"

echo "ISO image created: $ISO_NAME"
