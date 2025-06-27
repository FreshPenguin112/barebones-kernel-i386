.global idt_load
.type idt_load, @function
idt_load:
    mov %rdi, %rax    # IDT pointer in rdi (x86_64 ABI)
    lidt (%rax)
    ret
