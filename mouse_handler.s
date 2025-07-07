.global mouse_handler_asm

mouse_handler_asm:
    pusha
    call mouse_handler
    popa
    iret
