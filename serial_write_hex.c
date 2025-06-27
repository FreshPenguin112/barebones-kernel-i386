#include "serial.h"
#include <stdint.h>

// Print a 64-bit value as hexadecimal to the serial port
void serial_write_hex(uint64_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[17];
    buf[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        buf[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    serial_write_str(buf);
}
