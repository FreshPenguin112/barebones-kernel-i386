.section .data
.align 16
.global gdt64
.global gdt64_end

gdt64:
    /* Null descriptor */
    .quad 0x0000000000000000
    /* Kernel code segment: base=0, limit=0, 64-bit, present, ring 0 */
    .quad 0x00af9a000000ffff
    /* Kernel data segment: base=0, limit=0, present, ring 0 */
    .quad 0x00af92000000ffff
    /* TSS descriptor (16 bytes, filled in by C) */
    .quad 0x0000000000000000
    .quad 0x0000000000000000

gdt64_end:

.section .bss
.align 16
.global tss64
.global tss64_end

tss64:
    .skip 104
    /* IST1 (interrupt stack table) will be set by C */
tss64_end:

.section .text
.global gdt_load
.type gdt_load, @function
/*
 * void gdt_load(void);
 * Loads GDT and TSS, sets segment registers.
 */
gdt_load:
    lea gdt64(%rip), %rax
    mov $((gdt64_end-gdt64)-1), %ecx
    push %rcx
    push %rax
    lea 8(%rsp), %rdx
    lgdt (%rdx)
    add $16, %rsp
    /* Reload segment registers */
    mov $0x10, %ax  /* data segment selector */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs
    /* Far jump to reload CS */
    pushq $0x08
    lea 1f(%rip), %rax
    push %rax
    lfqjmp:
    lretq
1:
    /* Load TSS */
    mov $0x18, %ax
    ltr %ax
    ret
