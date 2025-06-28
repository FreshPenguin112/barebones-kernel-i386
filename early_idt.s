.section .data
.align 16
.global early_idt
.global early_idt_ptr

early_idt:
    /* 16 bytes: one entry, all zeros (null handler) */
    .zero 16

early_idt_ptr:
    .word 15      # limit (16 bytes - 1)
    .quad early_idt

.section .text
.global early_idt_load
.type early_idt_load, @function
/*
 * void early_idt_load(void);
 * Loads a minimal IDT with a single null entry.
 */
early_idt_load:
    lea early_idt_ptr(%rip), %rax
    lidt (%rax)
    ret
