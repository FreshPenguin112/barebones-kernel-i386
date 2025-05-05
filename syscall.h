#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h> // For NULL

// Syscall numbers
#define SYSCALL_PRINT 0
#define SYSCALL_EXIT 1
#define SYSCALL_GETCHAR 2
#define SYSCALL_UPTIME_MS 3
#define SYSCALL_RANDOM_FLOAT 4
#define SYSCALL_PUTCHAR 5
#define SYSCALL_READLINE 6
#define SYSCALL_ANSI_PRINT 7
#define SYSCALL_RANDOM_U32 8
#define SYSCALL_RANDOM_I32 9
#define SYSCALL_RANDOM_DOUBLE 10

// Helper macros for up to 4 arguments, auto-casting to void*
#define _syscall4(n, a1, a2, a3) syscall_impl((n), (void *)(a1), (void *)(a2), (void *)(a3))
#define _syscall3(n, a1, a2) syscall_impl((n), (void *)(a1), (void *)(a2), NULL)
#define _syscall2(n, a1) syscall_impl((n), (void *)(a1), NULL, NULL)
#define _syscall1(n) syscall_impl((n), NULL, NULL, NULL)

// Count arguments and dispatch to the right macro
#define _GET_SYSCALL(_1, _2, _3, _4, NAME, ...) NAME
#define syscall(...) _GET_SYSCALL(__VA_ARGS__, _syscall4, _syscall3, _syscall2, _syscall1)(__VA_ARGS__)

// Inline syscall implementation
static inline void syscall_impl(int syscall_number, void *arg1, void *arg2, void *arg3)
{
    asm volatile(
        "int $0x80"
        :
        : "a"(syscall_number), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory");
}

// Syscall dispatcher (kernel-side)
void syscall_dispatcher(int syscall_number, void *arg1, void *arg2, void *arg3);

#endif