.global idt_load
idt_load:
    mov %rdi, %rax      # IDT pointer is in rdi (System V AMD64 ABI)
    lidt (%rax)         # Load the IDT (10 bytes: 2-byte limit, 8-byte base)
    ret
