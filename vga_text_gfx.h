#ifndef VGA_TEXT_GFX_H
#define VGA_TEXT_GFX_H
#include <stdint.h>

// Draw a single 8x8 character at (x, y) in VGA graphics mode
void vga_draw_char8x8(int x, int y, char c, uint8_t color);
// Draw a string at (x, y)
void vga_draw_string8x8(int x, int y, const char *s, uint8_t color);
// Draw a block cursor at (x, y)
void vga_draw_cursor8x8(int x, int y, uint8_t color);

#endif
