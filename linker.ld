ENTRY(_start)
OUTPUT_FORMAT(pei-x86-64)

PHDRS {
    headers PT_PHDR PHDRS;
    text PT_LOAD FILEHDR PHDRS;
    data PT_LOAD;
    dynamic PT_DYNAMIC;
}

SECTIONS {
    . = 0x100000;
    
    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)
    } :text
    
    .rodata : ALIGN(4K) { *(.rodata*) } :text
    .data : ALIGN(4K) { *(.data*) } :data
    .bss : ALIGN(4K) { 
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } :data
    
    /DISCARD/ : { *(.eh_frame) }
}