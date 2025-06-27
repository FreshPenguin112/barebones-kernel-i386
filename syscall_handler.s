.section .text
.global syscall_handler_asm
.type syscall_handler_asm, @function

/* x86_64 syscall handler stub for Limine */
syscall_handler_asm:
    /* Save registers */
    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %r8
    push %r9
    push %r10
    push %r11

    /* Call C handler: syscall_handler(rdi, rsi, rdx) */
    call syscall_handler

    /* Restore registers */
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi

    /* Return from syscall */
    sysretq
