.global keyboard_handler_asm
keyboard_handler_asm:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    call keyboard_handler
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    mov $0x20, %al
    out %al, $0x20
    iret
