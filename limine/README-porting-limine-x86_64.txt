# Porting Notes: i386 to x86_64 Limine

1. Build system now uses GNUmakefile and x86_64-elf-gcc. See GNUmakefile for details.
2. Limine bootloader is used. See limine/limine.conf and get-deps script.
3. All assembly files must be ported to x86_64 ABI (start.s, syscall_handler.s, timer_handler.s, idt_load.s).
4. Linker script must be updated for x86_64 and Limine (see Limine template for reference).
5. Kernel entry point should be renamed to kmain and Limine requests set up in C (see Limine template main.c).
6. All C code must use 64-bit types and pointers. Remove i386-specific code (GDT, IDT, etc.) and replace with x86_64 versions.
7. QEMU scripts must use qemu-system-x86_64 and boot from ISO (see GNUmakefile run target).
8. Userland and ELF loader must be updated for 64-bit ELF if you want to run 64-bit user programs.

Start by porting start.s to x86_64 and setting up Limine requests in C. See https://github.com/limine-bootloader/limine-c-template-x86-64 for reference.
