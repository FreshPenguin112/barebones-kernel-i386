.global _start
.section .text
_start:
    mov %rsp, %rax
    and $~0xF, %rax
    mov %rax, %rsp
    call main
    hlt
