.section .text
.global syscall_handler_asm
.type syscall_handler_asm, @function

# Proper int $0x80 handler for x86_64 (software interrupt, not syscall/sysret)
syscall_handler_asm:
    # Serial debug: output 'S' for syscall entry (before any pushes)
    mov $0x53, %al        # 'S'
    mov $0x3F8, %dx
    out %al, %dx
    mov %rsp, %rax        # Print stack pointer (entry, true value)
    call print_hex_asm
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
    sub $8, %rsp         # align stack to 16 bytes before call
    mov %rax, %rdi       # syscall_number (from rax)
    mov %rbx, %rsi       # arg1
    mov %rcx, %rdx       # arg2
    mov %rdx, %rcx       # arg3 (move up for C calling convention)
    call syscall_dispatcher
    mov %rsp, %rax        # Print stack pointer (exit)
    call print_hex_asm
    # Serial debug: output 's' for syscall exit
    mov $0x73, %al        # 's'
    mov $0x3F8, %dx
    out %al, %dx
    add $8, %rsp
    # Check stack alignment before iretq
    mov %rsp, %rax
    and $0xF, %rax
    cmp $0, %rax
    jne .bad_align
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
.bad_align:
    mov $0x41, %al  # 'A' for alignment error
    mov $0x3F8, %dx
    out %al, %dx
    hlt

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
