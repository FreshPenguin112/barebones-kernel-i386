for src in userland/*.c; do
    fname=$(basename "$src" .c)
    if [ "$(echo "$fname" | cut -c1)" != "_" ]; then
        i686-elf-gcc -ffreestanding -nostdlib -g -Ttext=0x800000 string_utils.c "$src" -o "initfs/$fname.elf"
    fi
done