.section .text
.global default_int_handler_errcode_asm
.type default_int_handler_errcode_asm, @function

default_int_handler_errcode_asm:
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

    # Print only error code to serial (vector number is not on stack)
    mov $0x3F8, %dx
    movq 120(%rsp), %rax   # error code (top of stack after pushes)
    call print_hex_asm
    mov $'\n', %al
    out %al, %dx

    add $8, %rsp   # pop error code
    mov $0xBEEF, %rax
    hlt

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

# Helper: print 16-hex digits in %rax to serial (COM1)
print_hex_asm:
    push %rcx
    push %rsi
    mov $16, %rcx
    mov $0x3F8, %rsi
.print_hex_loop:
    mov %rax, %rdi
    shl $4, %rdi
    shr $60, %rdi
    add $'0', %dil
    cmp $'9', %dil
    jbe .no_alpha
    add $7, %dil
.no_alpha:
    mov %dil, %al
    out %al, %dx
    shl $4, %rax
    loop .print_hex_loop
    pop %rsi
    pop %rcx
    ret
