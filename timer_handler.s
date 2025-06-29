.global timer_handler_asm
.type timer_handler_asm, @function
timer_handler_asm:
    // Send EOI to PIC
    movb $0x20, %al
    out   %al, $0x20
    iretq
