.global idt_load
idt_load:
    movl 4(%esp), %eax    # Load the IDT pointer address from the stack
    lidt (%eax)           # Load the IDT
    ret
