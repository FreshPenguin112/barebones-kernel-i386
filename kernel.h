#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>

extern int vga_in_graphics_mode;
extern volatile int user_program_exited;
extern volatile uint32_t timer_ticks;

void kernel_print(const char *s);
void kernel_print_ansi(const char *text, const char *fg_name, const char *bg_name);
void kernel_putc(char c);
void kernel_putc_ansi(const char text, const char *fg_name, const char *bg_name);
void ansi_clearhome(void);
void vga_set_mode(uint8_t mode);
void vga_put_pixel(int x, int y, uint8_t color);
void term_handle_ansi_code(int code);
uint8_t ansi_to_vga_color(int code);

void serial_write_str(const char *s);

// Serial logging
void kernel_log(const char *msg);
void kernel_log_hex(const char *prefix, int value);

void pic_remap(void);
#endif
