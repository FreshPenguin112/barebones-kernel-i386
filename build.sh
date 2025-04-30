gcc/bin/i686-elf-gcc -std=gnu99 -ffreestanding -g -c *.s *.c
gcc/bin/i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld *.o -o mykernel.elf -lgcc
