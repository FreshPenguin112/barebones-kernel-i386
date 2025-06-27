#include <stddef.h>
#include <stdint.h>
#include "shell.h"
#include "serial.h"
#include "qemu_utils.h"
#include "tarfs.h"
#include "syscall.h"
#include "idt.h"
#include "string_utils.h"
#include "pit.h"
#include "io.h"
#include "multiboot.h"
#include "framebuffer.h"

volatile int user_program_exited = 0;
volatile uint32_t timer_ticks = 0;

// -----------------------------------------------------------------------------
// Serial (COM1) setup
// -----------------------------------------------------------------------------
// #define COM1_PORT 0x3F8

/*
void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB
    outb(COM1_PORT + 0, 0x03);    // Divisor low = 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00);    // Divisor high
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, 1 stop bit
    outb(COM1_PORT + 2, 0xC7);    // FIFO enabled, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_is_transmit_empty()) {}
    outb(COM1_PORT, c);
}
*/

void serial_write_str(const char *s)
{
    for (size_t i = 0; s[i]; i++)
        serial_write_char(s[i]);
}

// Unified putc for serial only
void kernel_putc(char c)
{
    serial_write_char(c);
}

// Minimal kernel_print for serial only (no ANSI)
void kernel_print(const char *s)
{
    for (size_t i = 0; s[i]; i++)
        kernel_putc(s[i]);
}

// Dummy stubs for removed ANSI/color functions (for compatibility)
void kernel_print_ansi(const char *text, const char *fg_name, const char *bg_name)
{
    kernel_print(text);
}
void kernel_putc_ansi(const char text, const char *fg_name, const char *bg_name)
{
    kernel_putc(text);
}
void ansi_clear(void) {}
void ansi_home(void) {}
void ansi_clearhome(void) {}

// -----------------------------------------------------------------------------
// Syscall handler
// -----------------------------------------------------------------------------
void (*elf_exit_callback)(void) = 0;

extern void syscall_handler_asm();

void syscall_handler(int syscall_number, void *arg1, void *arg2, void *arg3)
{
    syscall_dispatcher(syscall_number, arg1, arg2, arg3);
}

// Set up the syscall interrupt (int 0x80)
void setup_syscalls()
{
    idt_set_gate(0x80, (uint32_t)syscall_handler_asm, 0x08, 0x8E);
}

// -----------------------------------------------------------------------------
// Timer handler
// -----------------------------------------------------------------------------
extern void timer_handler_asm(void);

void timer_handler(void)
{
    timer_ticks++;
    outb(0x20, 0x20);
}

// -----------------------------------------------------------------------------
// Kernel entry point
// -----------------------------------------------------------------------------
void kernel_main(uint32_t magic, uint32_t mbi_addr)
{
    framebuffer_init_limine();

    serial_init();
    idt_init();
    pit_init(1000);
    idt_set_gate(32, (uint32_t)timer_handler_asm, 0x08, 0x8E);
    setup_syscalls();
    asm volatile("sti");
    extern unsigned char initfs_tar[];
    tarfs_init((const char *)initfs_tar);

    // VGA graphics test: draw a white box in the center
    extern void vga_set_mode(void);
    extern void vga_text_mode(void);
    extern void vga_plot_pixel(int x, int y, uint8_t color);
    vga_set_mode();
    // Draw a white box in the center using new abstraction
    for (int y = 40; y < 160; y++)
        for (int x = 60; x < 260; x++)
            vga_plot_pixel(x, y, 7); // white
    uint32_t start = timer_ticks;
    while ((timer_ticks - start) < 5000) {}
    vga_text_mode();

    shell_init();
    shell_run();
    for (;;)
    {
        asm volatile("hlt");
    }
}