#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// Serial port functions
void serial_init(void);
char serial_read_char(void);
void serial_write_char(char c);
void serial_write_str(const char *s);
void serial_write_hex(uint64_t value);
int serial_is_transmit_empty(void);
int serial_has_received(void);

#endif