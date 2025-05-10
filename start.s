.extern _start          

.global _startkernel    

.section .text

_startkernel:

    mov $stack_top, %rsp

    cld

    call _start

halt_loop:
    cli                 
    hlt                 
    jmp halt_loop       

.section .bss
.align 16
stack_bottom:
.skip 16384            
stack_top:

.section .data
.global bootboot
bootboot:
    .quad 0  # Reserve space for BOOTBOOT pointer

.section .text
.global _start
_start:
    # Save BOOTBOOT address from RDI to bootboot symbol
    movabs $bootboot, %rax
    mov %rdi, (%rax)

    # Jump to kernel main
    call kernel_main