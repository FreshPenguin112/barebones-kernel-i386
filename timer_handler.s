.section .text
.global timer_handler_asm

timer_handler_asm:
    pusha
    call timer_handler
    popa
    iret
