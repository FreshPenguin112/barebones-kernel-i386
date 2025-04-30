rm -f *.o userland/*.o initfs/*.o initfs/*.elf initfs.tar initramfs.c
for src in userland/*.c; do
    fname=$(basename "$src" .c)
    if [ "$(echo "$fname" | cut -c1)" != "_" ]; then
        gcc/bin/i686-elf-gcc -ffreestanding -nostdlib -g -Ttext=0x800000 string_utils.c "$src" -o "initfs/$fname.elf"
    fi
done
#sleep 2
tar -cf initfs.tar -C initfs .
xxd -i initfs.tar > initramfs.c
#sleep 2
gcc/bin/i686-elf-gcc -std=gnu99 -ffreestanding -g -c *.s *.c
gcc/bin/i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld *.o -o mykernel.elf -lgcc
gcc/bin/i686-elf-strip --strip-all initfs/*.elf mykernel.elf