#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h> // For NULL

// Syscall numbers
#define SYSCALL_PRINT      0
#define SYSCALL_EXIT       1
#define SYSCALL_GETCHAR    2
#define SYSCALL_TIME_MS    3
#define SYSCALL_RANDOM     4
#define SYSCALL_PUTCHAR 5

// Helper macro to select the correct syscall_impl variant
#define _syscall3(n, a1, a2) syscall_impl((n), (a1), (a2))
#define _syscall2(n, a1)     syscall_impl((n), (a1), NULL)
#define _syscall1(n)         syscall_impl((n), NULL, NULL)

// Count arguments and dispatch to the right macro
#define _GET_SYSCALL(_1, _2, _3, NAME, ...) NAME
#define syscall(...) _GET_SYSCALL(__VA_ARGS__, _syscall3, _syscall2, _syscall1)(__VA_ARGS__)

// Inline syscall implementation
static inline void syscall_impl(int syscall_number, void* arg1, void* arg2) {
    asm volatile (
        "int $0x80"
        :
        : "a"(syscall_number), "b"(arg1), "c"(arg2)
        : "memory"
    );
}

// Syscall dispatcher (kernel-side)
void syscall_dispatcher(int syscall_number, void* arg1, void* arg2);

#endif