#include "serial.h"
#include "exceptions.h"

void serial_write_string(const char *s) {
    for (; *s; ++s) serial_write_char(*s);
}

void serial_write_hex(uint64_t n) {
    char buf[17];
    for (int i = 15; i >= 0; --i) {
        int v = (n >> (i * 4)) & 0xF;
        buf[15 - i] = v < 10 ? '0' + v : 'A' + (v - 10);
    }
    buf[16] = 0;
    serial_write_string("0x");
    serial_write_string(buf);
}

void exception_dispatcher(uint64_t vector, uint64_t error_code) {
    serial_write_string("[EXC] Vector: ");
    serial_write_hex(vector);
    serial_write_string("  Error: ");
    serial_write_hex(error_code);
    serial_write_string("\r\n");
    for (;;) __asm__ volatile ("hlt");
}
