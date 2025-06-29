#include "serial.h"
#include "io.h"

#define COM1 0x3F8

void serial_init(void)
{
    outb(COM1 + 1, 0x00); // Disable interrupts
    outb(COM1 + 3, 0x80); // Enable DLAB
    outb(COM1 + 0, 0x03); // Set divisor lo byte (38400 baud)
    outb(COM1 + 1, 0x00); // Set divisor hi byte
    outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7); // Enable FIFO with 14-byte threshold
    outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

char serial_read_char(void)
{
    while (!(inb(COM1 + 5) & 0x01))
        ;
    return inb(COM1);
}

int serial_has_received(void)
{
    return inb(COM1 + 5) & 0x01;
}

int serial_is_transmit_empty(void)
{
    return inb(COM1 + 5) & 0x20;
}

void serial_write_char(char c)
{
    while (!serial_is_transmit_empty())
        ;
    outb(COM1, c);
}

void serial_print_hex(unsigned int value) {
    char buf[9];
    buf[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        unsigned int nibble = value & 0xF;
        buf[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        value >>= 4;
    }
    serial_write_str(buf);
}

void serial_print_dec(unsigned int value) {
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    if (value == 0) {
        buf[i] = '0';
        serial_write_str(&buf[i]);
        return;
    }
    while (value > 0 && i >= 0) {
        buf[i--] = '0' + (value % 10);
        value /= 10;
    }
    serial_write_str(&buf[i+1]);
}

void serial_write_str(const char *s)
{
    for (uint32_t i = 0; s[i]; i++)
        serial_write_char(s[i]);
}