.global keyboard_handler_asm

keyboard_handler_asm:
    pusha
    call keyboard_handler
    popa
    iret
