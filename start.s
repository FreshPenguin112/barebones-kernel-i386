/* Limine boot header for x86_64, must be in a .limine_reqs section and 16-byte aligned */
.section .limine_reqs, "a"
    .align 16
    # Limine Framebuffer request ID: 0x3e7e279702be32af, 0xca1c4f3bd1280cee, 0xe1cb0fc25f46ea3d, 0x447b7e6a740fe466
    .quad 0x3e7e279702be32af
    .quad 0xca1c4f3bd1280cee
    .quad 0xe1cb0fc25f46ea3d
    .quad 0x447b7e6a740fe466
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0

# We declare the 'kernel_main' label as being external to this file.
.extern kernel_main

# We declare the '_start' label as global (accessible from outside this file), since the linker will need to know where it is.
.global _start

.section .text
    .align 16
_start:
    # Limine guarantees long mode, paging, and a valid stack.
    # Just jump to kernel_main (or your C entry point)
    call kernel_main
    cli
    hlt
    jmp .

