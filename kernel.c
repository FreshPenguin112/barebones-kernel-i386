#include <stddef.h>
#include <stdint.h>
#include "shell.h" // Change to use quotes instead of angle brackets
#include "serial.h"
#include "qemu_utils.h"

// -----------------------------------------------------------------------------
// VGA text‐mode state
// -----------------------------------------------------------------------------
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F; // white on black

// -----------------------------------------------------------------------------
// I/O port helpers
// -----------------------------------------------------------------------------
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

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
// VGA text‐mode routines (unchanged)
// -----------------------------------------------------------------------------
void term_init(void)
{
    for (int r = 0; r < VGA_ROWS; r++)
    {
        for (int c = 0; c < VGA_COLS; c++)
        {
            vga_buffer[r * VGA_COLS + c] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_row = term_col = 0;
}

void term_setcolor(uint8_t fg, uint8_t bg)
{
    term_color = (bg << 4) | (fg & 0x0F);
}

void term_reset_color(void)
{
    term_setcolor(0x0F, 0x00);
}

uint8_t ansi_to_vga_color(int code)
{
    // maps 30–37 or 40–47 → VGA palette
    switch (code)
    {
    case 30:
        return 0;
    case 31:
        return 4;
    case 32:
        return 2;
    case 33:
        return 6;
    case 34:
        return 1;
    case 35:
        return 5;
    case 36:
        return 3;
    case 37:
        return 7;
    default:
        return 7;
    }
}

void term_handle_ansi_code(int code)
{
    if (code == 0)
    {
        term_reset_color();
    }
    else if (code >= 30 && code <= 37)
    {
        // set fg only
        term_setcolor(ansi_to_vga_color(code), term_color >> 4);
    }
    else if (code >= 40 && code <= 47)
    {
        // set bg only
        term_setcolor(term_color & 0x0F, ansi_to_vga_color(code - 10));
    }
}

void term_putc_vga(char c)
{
    if (c == '\n')
    {
        term_col = 0;
        term_row++;
    }
    else
    {
        vga_buffer[term_row * VGA_COLS + term_col] =
            ((uint16_t)term_color << 8) | c;
        term_col++;
    }
    if (term_col >= VGA_COLS)
    {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS)
    {
        term_row = 0;
    }
}

// -----------------------------------------------------------------------------
// Unified “putc” for serial + VGA
// -----------------------------------------------------------------------------
void kernel_putc(char c)
{
    serial_write_char(c);
    term_putc_vga(c);
}

// send ESC [ … m to serial & update VGA; `seq` is digits only, no ‘m’
void kernel_handle_ansi_and_putc(const char *seq, size_t len)
{
    serial_write_str("\033[");
    for (size_t i = 0; i < len; i++)
        serial_write_char(seq[i]);
    serial_write_char('m');

    // parse the integer
    int code = 0;
    for (size_t i = 0; i < len; i++)
        code = code * 10 + (seq[i] - '0');
    term_handle_ansi_code(code);
}

// print a plain C string, handling only “ESC[…m” sequences
void kernel_print(const char *s)
{
    for (size_t i = 0; s[i]; i++)
    {
        if (s[i] == '\033' && s[i + 1] == '[')
        {
            size_t j = i + 2;
            while (s[j] && s[j] != 'm')
                j++;
            if (s[j] == 'm')
            {
                kernel_handle_ansi_and_putc(&s[i + 2], j - (i + 2));
                i = j;
                continue;
            }
        }
        kernel_putc(s[i]);
    }
}

// -----------------------------------------------------------------------------
// Minimal strcmp (no libc)
// -----------------------------------------------------------------------------
static int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *(unsigned char *)a - *(unsigned char *)b;
}

// -----------------------------------------------------------------------------
// Map human names → ANSI codes
// -----------------------------------------------------------------------------
static int ansi_fg_code(const char *c)
{
    if (!strcmp(c, "black"))
        return 30;
    else if (!strcmp(c, "red"))
        return 31;
    else if (!strcmp(c, "green"))
        return 32;
    else if (!strcmp(c, "yellow"))
        return 33;
    else if (!strcmp(c, "blue"))
        return 34;
    else if (!strcmp(c, "magenta"))
        return 35;
    else if (!strcmp(c, "cyan"))
        return 36;
    else if (!strcmp(c, "white"))
        return 37;
    else
        return -1;
}
static int ansi_bg_code(const char *c)
{
    if (!strcmp(c, "black"))
        return 40;
    else if (!strcmp(c, "red"))
        return 41;
    else if (!strcmp(c, "green"))
        return 42;
    else if (!strcmp(c, "yellow"))
        return 43;
    else if (!strcmp(c, "blue"))
        return 44;
    else if (!strcmp(c, "magenta"))
        return 45;
    else if (!strcmp(c, "cyan"))
        return 46;
    else if (!strcmp(c, "white"))
        return 47;
    else
        return -1;
}

// format a two‐digit code into `out[]`, return length (always 2 here)
static size_t fmt_code(char out[2], int code)
{
    out[0] = '0' + (code / 10);
    out[1] = '0' + (code % 10);
    return 2;
}

// -----------------------------------------------------------------------------
// High-level ANSI wrapper
// -----------------------------------------------------------------------------
void kernel_print_ansi(const char *text,
                       const char *fg_name,
                       const char *bg_name)
{
    char buf[2];
    size_t len;
    int fg = ansi_fg_code(fg_name);
    int bg = ansi_bg_code(bg_name);

    if (fg >= 0)
    {
        len = fmt_code(buf, fg);
        kernel_handle_ansi_and_putc(buf, len);
    }
    if (bg >= 0)
    {
        len = fmt_code(buf, bg);
        kernel_handle_ansi_and_putc(buf, len);
    }

    // now the text
    for (const char *p = text; *p; p++)
        kernel_putc(*p);

    // reset if we changed anything
    if (fg >= 0 || bg >= 0)
        kernel_handle_ansi_and_putc("0", 1);
}

// -----------------------------------------------------------------------------
// ANSI screen/cursor controls
// -----------------------------------------------------------------------------
void ansi_clear(void)
{
    serial_write_str("\033[2J");
    term_init();
}
void ansi_home(void)
{
    serial_write_str("\033[H");
    term_row = term_col = 0;
}
void ansi_clearhome(void)
{
    ansi_clear();
    ansi_home();
}

// -----------------------------------------------------------------------------
// Kernel entry point
// -----------------------------------------------------------------------------
void kernel_main(void)
{
    serial_init();
    term_init();

    // Clear & home
    ansi_clearhome();

    // Initialize and run shell
    shell_init();
    shell_run();

    // Halt the CPU
    for (;;)
    {
        asm volatile("hlt");
    }
}