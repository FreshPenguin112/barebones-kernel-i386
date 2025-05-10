.section .text
.global syscall_handler_asm

syscall_handler_asm:
    # Save fs and gs (if needed)
    push %fs
    push %gs

    # Move syscall number and arguments into registers for C call
    # Syscall number in rax, arguments in rdi, rsi, rdx, r10, r8, r9
    movq %rax, %rdi    # First argument: syscall_number
    movq %rdi, %rsi    # Second argument: arg1
    movq %rsi, %rdx    # Third argument: arg2

    call syscall_handler

    # Restore fs and gs
    pop %gs
    pop %fs

    iretq               # Use iretq for 64-bit return