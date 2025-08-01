#ifndef SERIAL_H
#define SERIAL_H

// Serial port functions
void serial_init(void);
char serial_read_char(void);
void serial_write_char(char c);
int serial_is_transmit_empty(void);
int serial_has_received(void);
void serial_print_hex(unsigned int value);
void serial_print_dec(unsigned int value);
void serial_write_str(const char *s);

#endif