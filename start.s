.extern kernel_main
.global start

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:

.section .text
start:
    lea stack_top(%rip), %rsp
    and $~0xF, %rsp      # 16-byte align
    call kernel_main

hang:
    cli
    hlt
    jmp hang
