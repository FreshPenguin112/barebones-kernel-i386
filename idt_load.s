.global idt_load
idt_load:
    movq %rdi, %rax      # First argument in rdi (x86_64 ABI)
    lidt (%rax)          # Load the IDT
    ret
