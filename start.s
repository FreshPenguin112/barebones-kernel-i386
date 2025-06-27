// We declare the 'kernel_main' label as being external to this file.
// That's because it's the name of the main C function in 'kernel.c'.
.extern kernel_main
 
// We declare the 'start' label as global (accessible from outside this file), since the linker will need to know where it is.
// In a bit, we'll actually take a look at the code that defines this label.
.global start
 
// This section contains our actual assembly code to be run when our kernel loads
.section .text
    .global _start
    .type _start, @function
    .extern _bss_start
    .extern _bss_end
    .extern kmain

_start:
    /* Set up stack pointer (8MB stack at 0xFFFFFFFF80800000) */
    mov $0xFFFFFFFF80800000, %rsp

    /* Clear BSS section */
    mov $_bss_start, %rdi
    mov $_bss_end, %rcx
    sub %rdi, %rcx
    xor %rax, %rax
    rep stosb

    /* Call kernel main (kmain) */
    call kmain

.hang:
    hlt
    jmp .hang

    .size _start, .-_start

