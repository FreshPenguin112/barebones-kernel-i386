i686-elf-gcc -g -std=gnu99 -ffreestanding -g -c *.s *.c
i686-elf-gcc -g -ffreestanding -nostdlib -g -T linker.ld *.o -o mykernel.elf -lgcc
