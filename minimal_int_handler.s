.section .text
.global minimal_int_handler_asm
minimal_int_handler_asm:
    push %rax
    mov $0x4D, %al      # 'M'
    mov $0x3F8, %dx
    out %al, %dx
    pop %rax
    mov $0x20, %al      # EOI for PIC
    out %al, $0x20
    iretq
