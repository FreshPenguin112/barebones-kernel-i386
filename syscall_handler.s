.section .text
.global syscall_handler_asm

# x86_64 syscall handler: expects syscall number in rax, args in rdi, rsi, rdx
syscall_handler_asm:
    push %rbp
    mov %rsp, %rbp
    # Save registers that may be clobbered
    push %rdi
    push %rsi
    push %rdx
    # Call C handler: int syscall_handler(uint64_t num, uint64_t arg1, uint64_t arg2)
    mov %rax, %rdi   # syscall number
    mov %rdi, %rsi   # arg1
    mov %rsi, %rdx   # arg2
    call syscall_handler
    # Restore registers
    pop %rdx
    pop %rsi
    pop %rdi
    pop %rbp
    ret
