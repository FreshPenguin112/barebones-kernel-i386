.section .text
.global syscall_handler_asm

syscall_handler_asm:
    # Save segment registers
    push %ds
    push %es
    push %fs
    push %gs

    push %eax              # Save syscall number
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    pop %eax               # Restore syscall number

    # Push syscall arguments for C (from userland: eax, ebx, ecx)
    push %ecx        # arg2
    push %ebx        # arg1
    push %eax        # syscall_number

    call syscall_handler
    add $12, %esp    # Clean up stack

    # Restore segment registers
    pop %gs
    pop %fs
    pop %es
    pop %ds

    iret
