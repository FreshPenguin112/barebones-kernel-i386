.section .text
// Exception stubs for vectors 0-31 (x86_64, GAS syntax)
// Each pushes error code if needed, then vector number, then calls exception_dispatcher

// Vectors with error code: 8, 10, 11, 12, 13, 14, 17
// Others: push dummy error code

// Macro for no-error-code exceptions
.macro ISR_NOERR n
    .globl isr\n
isr\n:
    pushq $0                # error code
    pushq $\n               # vector number
    popq %rdi               # vector -> rdi
    popq %rsi               # error code -> rsi
    call exception_dispatcher
    hlt
.endm

// Macro for error-code exceptions
.macro ISR_ERR n
    .globl isr\n
isr\n:
    pushq $\n               # vector number
    popq %rdi               # vector -> rdi
    popq %rsi               # error code -> rsi (already on stack from CPU)
    call exception_dispatcher
    hlt
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31
