#include "syscall.h"
#include "string_utils.h"
#include "qemu_utils.h"
#include "serial.h"
//#include "kernel.h" // for kernel_print, etc.
#include <stdint.h>

extern void kernel_print(const char* str);
extern volatile uint32_t timer_ticks; // You need a timer interrupt to increment this
extern void kernel_putc(char c);

typedef void (*syscall_handler_t)(void* arg1, void* arg2);

// Syscall handlers
static void syscall_print(void* arg1, void* arg2) {
    kernel_print((const char*)arg1);
}

static void syscall_exit(void* arg1, void* arg2) {
    kernel_print("Exiting program.\n");
    return;
    //qemu_halt_exit(0);
}

static void syscall_getchar(void* arg1, void* arg2) {
    if (arg1) {
        char c = serial_read_char();
        *(char*)arg1 = c;
    }
}

static void syscall_time_ms(void* arg1, void* arg2) {
    if (arg1) {
        *(uint32_t*)arg1 = timer_ticks; // or convert to ms if needed
    }
}

static uint32_t rand_seed = 123456789;
static void syscall_random(void* arg1, void* arg2) {
    // Simple LCG random number generator
    rand_seed = rand_seed * 1103515245 + 12345;
    if (arg1) *(uint32_t*)arg1 = (rand_seed >> 16) & 0x7FFF;
}

static void syscall_putchar(void* arg1, void* arg2) {
    kernel_putc((char)(uintptr_t)arg1);
}

// Syscall table
static syscall_handler_t syscall_table[] = {
    syscall_print,      // 0
    syscall_exit,       // 1
    syscall_getchar,    // 2
    syscall_time_ms,    // 3
    syscall_random,     // 4
    syscall_putchar     // 5
};

#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_handler_t))

// Syscall dispatcher
void syscall_dispatcher(int syscall_number, void* arg1, void* arg2) {
    if (syscall_number < 0 || syscall_number >= SYSCALL_COUNT) {
        kernel_print("Unknown syscall\n");
        return;
    }
    syscall_table[syscall_number](arg1, arg2);
}