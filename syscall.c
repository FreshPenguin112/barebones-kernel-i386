#include "syscall.h"
#include "string_utils.h"
#include "qemu_utils.h"
#include "serial.h"
#include "kernel.h"
#include <stdint.h>

extern void kernel_print(const char *str);
extern volatile uint32_t timer_ticks; // You need a timer interrupt to increment this
extern void kernel_putc(char c);

typedef void (*syscall_handler_t)(void *arg1, void *arg2, void *arg3);

// Syscall handlers
static void syscall_print(void *arg1, void *arg2, void *arg3)
{
    kernel_print((const char *)arg1);
}

static void syscall_exit(void *arg1, void *arg2, void *arg3)
{
    //kernel_print("Exiting program.\n");
    return;
    // qemu_halt_exit(0);
}

static void syscall_getchar(void *arg1, void *arg2, void *arg3)
{
    if (arg1)
    {
        char c = serial_read_char();
        *(char *)arg1 = c;
    }
}

static void syscall_uptime_ms(void *arg1, void *arg2, void *arg3)
{
    if (arg1)
    {
        *(uint32_t *)arg1 = timer_ticks; // or convert to ms if needed
    }
}

static uint32_t rand_seed = 123456789;

/* Advance the LCG state once */
static inline uint32_t lcg_next(void)
{
    // X_{n+1} = (1103515245 * X_n + 12345) mod 2^32
    // Standard “glibc” constants :contentReference[oaicite:3]{index=3}
    return (rand_seed = rand_seed * 1103515245u + 12345u);
}

static void syscall_random_u32(void *arg1, void *arg2, void *arg3) {
    if (arg1) {
        *(uint32_t *)arg1 = lcg_next();  // full 32 bits :contentReference[oaicite:4]{index=4}
    }
}

static void syscall_random_i32(void *arg1, void *arg2, void *arg3) {
    if (arg1) {
        *(int32_t *)arg1 = (int32_t)lcg_next();  // uniform over all int32_t :contentReference[oaicite:5]{index=5}
    }
}

static void syscall_random_double(void *arg1, void *arg2, void *arg3) {
    if (arg1) {
        uint32_t r = lcg_next();
        // r / 2^32 yields [0,1) exactly, since double has 52 mantissa bits :contentReference[oaicite:6]{index=6}
        *(double *)arg1 = (double)r / 4294967296.0;
    }
}

/*
 * syscall_random_float:
 *   Generates a pseudo‑random float in [0,1).
 *   arg1: pointer to float where result is stored.
 *   arg2, arg3: unused.
 */
static void syscall_random_float(void *arg1, void *arg2, void *arg3)
{
    /* advance LCG */
    rand_seed = rand_seed * 1103515245 + 12345;

    if (arg1)
    {
        /* take the high 15 bits, divide by 2^15 to get [0,1) */
        uint32_t v = (rand_seed >> 16) & 0x7FFF;
        *(float *)arg1 = v / 32768.0f;
    }
}

static void syscall_putchar(void *arg1, void *arg2, void *arg3)
{
    kernel_putc((char)(uintptr_t)arg1);
}

static void syscall_readline(void *arg1, void *arg2, void *arg3)
{
    // arg1: buffer, arg2: maxlen (as uintptr_t)
    char *buf = (char *)arg1;
    int maxlen = (int)(uintptr_t)arg2;
    int i = 0;
    while (i < maxlen - 1)
    {
        char c = serial_read_char();
        if (c == '\r' || c == '\n')
        {
            kernel_putc('\n');
            break;
        }
        if (c == 8 || c == 127)
        { // Backspace
            if (i > 0)
            {
                i--;
                kernel_print("\b \b");
            }
            continue;
        }
        kernel_putc(c);
        buf[i++] = c;
    }
    buf[i] = 0;
}

// Syscall handler
static void syscall_ansi_print(void *arg1, void *arg2, void *arg3)
{
    // Print a test string with the given colors
    int fg = (int)(uintptr_t)arg1;
    int bg = (int)(uintptr_t)arg2;
    // Convert color codes to color names if needed, or use as string
    // For now, just print a test string
    kernel_print_ansi("ANSI PRINT\n", "white", "black");
}

// Syscall handler
static void syscall_ansi_color(void *arg1, void *arg2, void *arg3)
{
    // Set color for subsequent prints (not implemented, so just print test)
    int fg = (int)(uintptr_t)arg1;
    int bg = (int)(uintptr_t)arg2;
    kernel_print_ansi("ANSI COLOR\n", "white", "black");
}

// Graphics syscall handlers
extern void vga_set_mode(uint8_t mode);
extern void vga_put_pixel(int x, int y, uint8_t color);

static void syscall_set_mode(void *arg1, void *arg2, void *arg3) {
    vga_set_mode((uint8_t)(uintptr_t)arg1);
}

static void syscall_put_pixel(void *arg1, void *arg2, void *arg3) {
    vga_put_pixel((int)(uintptr_t)arg1, (int)(uintptr_t)arg2, (uint8_t)(uintptr_t)arg3);
}

// Syscall table
static syscall_handler_t syscall_table[] = {
    syscall_print,         // 0
    syscall_exit,          // 1
    syscall_getchar,       // 2
    syscall_uptime_ms,     // 3
    syscall_random_float,  // 4
    syscall_putchar,       // 5
    syscall_readline,      // 6
    syscall_ansi_color,    // 7
    syscall_random_u32,    // 8
    syscall_random_i32,    // 9
    syscall_random_double, // 10
    [SYSCALL_SET_MODE] = syscall_set_mode,
    [SYSCALL_PUT_PIXEL] = syscall_put_pixel
};
#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_handler_t))

// Syscall dispatcher
void syscall_dispatcher(int syscall_number, void *arg1, void *arg2, void *arg3)
{
    if (syscall_number < 0 || syscall_number >= SYSCALL_COUNT)
    {
        kernel_print("Unknown syscall\n");
        return;
    }
    syscall_table[syscall_number](arg1, arg2, arg3);
}