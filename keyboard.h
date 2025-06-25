#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
int keyboard_has_char(void);
// keyboard_read_char: returns 0 if no char available (non-blocking)
char keyboard_read_char(void);
void keyboard_handler(void);
void keyboard_clear_buffer(void);

// Special key codes for arrow keys, Home/End, Delete
#define KEY_UP    0x80
#define KEY_DOWN  0x81
#define KEY_RIGHT 0x82
#define KEY_LEFT  0x83
#define KEY_HOME  0x84
#define KEY_END   0x85
#define KEY_DEL   0x7F

#endif
