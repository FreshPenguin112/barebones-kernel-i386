# Compile kernel
i686-elf-gcc -std=gnu99 -ffreestanding -g -c *.s *.c
i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld *.o -o mykernel.elf -lgcc

# Compile userland programs, skip files starting with '_'
for src in userland/*.c; do
    fname=$(basename "$src" .c)
    if [ "$(echo "$fname" | cut -c1)" != "_" ]; then
        i686-elf-gcc -ffreestanding -nostdlib -g -Ttext=0x800000 string_utils.c "$src" -o "initfs/$fname.elf"
    fi
done