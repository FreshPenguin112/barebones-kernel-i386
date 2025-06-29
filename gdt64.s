// GDT for x86_64 with TSS descriptor
.section .data
.align 16
.global gdt64
.global gdt64_end
.global gdt64_ptr

gdt64:
    .quad 0x0000000000000000  # Null
    .quad 0x00af9a000000ffff  # Kernel code (0x08)
    .quad 0x00af92000000ffff  # Kernel data (0x10)
    .quad 0x0000000000000000  # TSS low (0x18)
    .quad 0x0000000000000000  # TSS high (0x20)
gdt64_end:

gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

// Function to load GDT
.global load_gdt64
load_gdt64:
    mov $0x3F8, %dx
    mov $'S', %al
    out %al, %dx
    mov %rsp, %rax
    mov $0x3F8, %dx
    mov $'R', %al
    out %al, %dx
    mov %rax, %rbx
    mov $0x3F8, %dx
    mov $'P', %al
    out %al, %dx
    lgdt gdt64_ptr
    mov $0x3F8, %dx
    mov $'G', %al
    out %al, %dx
    pushq $0x08                # Kernel code segment selector
    lea 1f(%rip), %rax         # Address of label 1
    mov $0x3F8, %dx
    mov $'J', %al
    out %al, %dx
    pushq %rax
    lretq                     # Far return to reload CS
1:
    mov $0x3F8, %dx
    mov $'X', %al
    out %al, %dx
    ret
