/* x86_64 Limine-compatible higher half kernel linker script */
ENTRY(_start)

/* Higher half base address for kernel */
KERNEL_BASE = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KERNEL_BASE;

    .text : ALIGN(4K) {
        *(.text*)
    }
    .rodata : ALIGN(4K) {
        *(.rodata*)
    }
    /* Place lowdata at 0x10000 physical, identity-mapped */
    .lowdata 0x10000 : AT(0x10000) {
        *(.lowdata)
    }
    .data : ALIGN(4K) {
        *(.data*)
    }
    .bss : ALIGN(4K) {
        *(.bss*)
        *(COMMON)
    }
}
