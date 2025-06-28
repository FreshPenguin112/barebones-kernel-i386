for src in userland/*.c; do
    fname=$(basename "$src" .c)
    if [ "$(echo "$fname" | cut -c1)" != "_" ]; then
        x86_64-elf-gcc -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -c userland/_start.s -o userland/_start.o
        x86_64-elf-gcc -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -mcmodel=kernel -ffreestanding -nostdlib -g -Ttext=0x800000 string_utils.c "$src" userland/_start.o -o "initfs/$fname.elf"
    fi
done