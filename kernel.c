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
#include "keyboard.h"
#include "kernel.h"

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

// -----------------------------------------------------------------------------
// Syscall handler additions
// -----------------------------------------------------------------------------
#define SYSCALL_SET_MODE 20
#define SYSCALL_PUT_PIXEL 21

void syscall_handler(int syscall_number, void *arg1, void *arg2, void *arg3)
{
    syscall_dispatcher(syscall_number, arg1, arg2, arg3);
}

// Set up the syscall interrupt (int 0x80)
extern void syscall_handler_asm(void);

void setup_syscalls()
{
    idt_set_gate(0x80, (uint32_t)syscall_handler_asm, 0x08, 0x8E);
}

// Provide a weak default for syscall_dispatcher if not already defined
__attribute__((weak)) void syscall_dispatcher(int syscall_number, void *arg1, void *arg2, void *arg3) {
    (void)syscall_number; (void)arg1; (void)arg2; (void)arg3;
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
void kernel_main(void)
{
    serial_init();
    keyboard_init(); // initialize keyboard

    idt_init();                                                // 1. Set up IDT
    pic_remap();                                               // 2. Remap and unmask PIC
    pit_init(1000);                                            // 3. Set up PIT
    extern void keyboard_handler_asm(void);
    idt_set_gate(33, (uint32_t)keyboard_handler_asm, 0x08, 0x8E); // 4. IRQ1 (keyboard)
    idt_set_gate(32, (uint32_t)timer_handler_asm, 0x08, 0x8E); // 5. IRQ0
    setup_syscalls();                                          // 6. Syscalls

    asm volatile("sti"); // 7. Enable interrupts

    extern unsigned char initfs_tar[];
    tarfs_init((const char *)initfs_tar);

    shell_init();
    shell_run();

    for (;;)
    {
        asm volatile("hlt");
    }
}