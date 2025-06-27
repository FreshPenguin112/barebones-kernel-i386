/* Limine x86_64 kernel linker script */
ENTRY(_start)

SECTIONS
{
    /* Kernel is loaded at 0xFFFFFFFF80000000 */
    . = 0xFFFFFFFF80000000;

    .text : ALIGN(2M) {
        *(.text .text.*)
    }
    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*)
    }
    .data : ALIGN(4K) {
        *(.data .data.*)
    }
    .bss : ALIGN(4K) {
        _bss_start = .;
        *(.bss .bss.* COMMON)
        _bss_end = .;
    }
    /* Limine requests section */
    .limine_requests : ALIGN(4K) {
        *(.limine_requests)
    }
    .limine_requests_start : ALIGN(4K) {
        *(.limine_requests_start)
    }
    .limine_requests_end : ALIGN(4K) {
        *(.limine_requests_end)
    }
}