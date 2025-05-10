.section .text.boot
.global _bootboot_header
.align 8

_bootboot_header:

    .ascii "BOOT"

    .long 64

    .byte 2

    .byte 1

    .byte 0

    .byte 0

    .quad 0                    
    .quad 0                    
    .quad 0                    
    .long 0                    
    .long 0                    
    .long 0                    
    .long 0                    

    .fill 8, 1, 0              
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    .quad 0                    
    