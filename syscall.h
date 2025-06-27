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
#define SYSCALL_VGA_SET_MODE  11
#define SYSCALL_VGA_TEXT_MODE 12
#define SYSCALL_VGA_PLOT_PIXEL 13
//#define SYSCALL_VGA_CLEAR 14

// Helper macros for up to 4 arguments, auto-casting to uintptr_t
#define _syscall4(n, a1, a2, a3) syscall_impl((n), (uintptr_t)(a1), (uintptr_t)(a2), (uintptr_t)(a3))
#define _syscall3(n, a1, a2) syscall_impl((n), (uintptr_t)(a1), (uintptr_t)(a2), (uintptr_t)0)
#define _syscall2(n, a1) syscall_impl((n), (uintptr_t)(a1), (uintptr_t)0, (uintptr_t)0)
#define _syscall1(n) syscall_impl((n), (uintptr_t)0, (uintptr_t)0, (uintptr_t)0)

#define _GET_SYSCALL(_1, _2, _3, _4, NAME, ...) NAME
#define syscall(...) _GET_SYSCALL(__VA_ARGS__, _syscall4, _syscall3, _syscall2, _syscall1)(__VA_ARGS__)

// Inline syscall implementation
static inline void syscall_impl(int syscall_number, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
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