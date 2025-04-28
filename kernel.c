#include <stddef.h>
#include <stdint.h>

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
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// -----------------------------------------------------------------------------
// Serial (COM1) setup
// -----------------------------------------------------------------------------
#define COM1_PORT 0x3F8

void serial_init() {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14‐byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_empty() {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    // wait until the transmitter is ready
    while (!serial_is_transmit_empty()) { }
    outb(COM1_PORT, c);
}

void serial_write_str(const char *s) {
    for (size_t i = 0; s[i]; i++) {
        serial_write_char(s[i]);
    }
}

// -----------------------------------------------------------------------------
// VGA text‐mode routines (unchanged)
// -----------------------------------------------------------------------------
void term_init() {
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            const size_t idx = row * VGA_COLS + col;
            vga_buffer[idx] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_row = term_col = 0;
}

void term_setcolor(uint8_t fg, uint8_t bg) {
    term_color = (bg << 4) | (fg & 0x0F);
}

void term_reset_color() {
    term_setcolor(0x0F, 0x00);
}

uint8_t ansi_to_vga_color(int code) {
    switch (code) {
    case 30: return 0;
    case 31: return 4;
    case 32: return 2;
    case 33: return 6;
    case 34: return 1;
    case 35: return 5;
    case 36: return 3;
    case 37: return 7;
    default: return 7;
    }
}

void term_handle_ansi_code(int code) {
    if (code == 0) {
        term_reset_color();
    } else if (code >= 30 && code <= 37) {
        term_setcolor(ansi_to_vga_color(code), term_color >> 4);
    } else if (code >= 40 && code <= 47) {
        term_setcolor(term_color & 0x0F, ansi_to_vga_color(code - 10));
    }
}

void term_putc_vga(char c) {
    if (c == '\n') {
        term_col = 0;
        term_row++;
    } else {
        const size_t idx = term_row * VGA_COLS + term_col;
        vga_buffer[idx] = ((uint16_t)term_color << 8) | c;
        term_col++;
    }
    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) {
        term_row = 0;  // simple wrap‐around
    }
}

// -----------------------------------------------------------------------------
// Unified “putc” that drives BOTH serial & VGA, preserving ANSI on serial
// -----------------------------------------------------------------------------
void kernel_putc(char c) {
    serial_write_char(c);
    term_putc_vga(c);
}

void kernel_handle_ansi_and_putc(const char *seq, size_t len) {
    // send the raw escape to serial:
    serial_write_str("\033[");
    for (size_t i = 0; i < len; i++) {
        serial_write_char(seq[i]);
    }
    serial_write_char('m');

    // parse for VGA
    int code = 0;
    for (size_t i = 0; i + 1 < len; i++) {
        code = code * 10 + (seq[i] - '0');
    }
    term_handle_ansi_code(code);
}

// print a NUL‐terminated string, handling ANSI for VGA & passing raw to serial
void kernel_print(const char *s) {
    for (size_t i = 0; s[i]; i++) {
        if (s[i] == '\033' && s[i+1] == '[') {
            // find the “m”
            size_t j = i + 2;
            while (s[j] && s[j] != 'm') j++;
            if (s[j] == 'm') {
                kernel_handle_ansi_and_putc(&s[i+2], j - (i+2));
                i = j;  // skip over the entire escape
                continue;
            }
        }
        kernel_putc(s[i]);
    }
}

// -----------------------------------------------------------------------------
// Your kernel entry point
// -----------------------------------------------------------------------------
void kernel_main() {
    serial_init();    // bring up COM1 @ 38400
    term_init();      // clear VGA text–mode buffer, reset term_row/term_col

    // 1) Clear the screen and home the cursor (ANSI):
    kernel_print("\033[2J\033[H");

    // 2) Reset the VGA hardware cursor to (0,0):
    outb(0x3D4, 0x0E);
    outb(0x3D5, 0x00);
    outb(0x3D4, 0x0F);
    outb(0x3D5, 0x00);

    // 3) Now print your test lines:
    kernel_print("Default colors: Hello, World!\n");
    kernel_print("\033[31mRed foreground\033[0m\n");
    kernel_print("\033[44mBlue background\033[0m\n");
    kernel_print("Back to normal.\n");

    for (;;) {
        asm volatile("hlt");
    }
}
