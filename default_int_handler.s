.section .text
.global default_int_handler_asm
default_int_handler_asm:
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rbp
    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %rbx
    push %rax
    sub $8, %rsp         # align stack to 16 bytes
    # Serial debug: output 'I' for generic interrupt
    mov $0x49, %al        # 'I'
    mov $0x3F8, %dx
    out %al, %dx
    mov $0xDEAD, %rax
    hlt
    add $8, %rsp
    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi
    pop %rbp
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    iretq
