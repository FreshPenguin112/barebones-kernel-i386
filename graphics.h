#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

// VGA graphics mode constants
#define VGA_GFX_WIDTH 320
#define VGA_GFX_HEIGHT 200

// Set VGA mode: 0x13 = graphics, 0x03 = text
void set_graphics_mode();
void set_text_mode();

// Draw a pixel at (x, y) with color (0-255)
void put_pixel(int x, int y, uint8_t color);

// Draw a horizontal line
void draw_hline(int x, int y, int length, uint8_t color);

// Draw a vertical line
void draw_vline(int x, int y, int length, uint8_t color);

// Draw a rectangle (outline)
void draw_rect(int x, int y, int w, int h, uint8_t color);

// Fill a rectangle
void fill_rect(int x, int y, int w, int h, uint8_t color);

// Draw a line (Bresenham)
void draw_line(int x0, int y0, int x1, int y1, uint8_t color);

// Draw a circle (outline)
void draw_circle(int xc, int yc, int r, uint8_t color);

// Fill a circle
void fill_circle(int xc, int yc, int r, uint8_t color);

// Clear the screen to a color
void clear_screen(uint8_t color);

// Draw an 8x8 pixel character
void draw_char8x8(int x, int y, char c, uint8_t color);

// Draw a string of characters using 8x8 pixel font
void draw_string8x8(int x, int y, const char *s, uint8_t color);

// Draw an 8x8 pixel cursor (for text mode)
void draw_cursor8x8(int x, int y, uint8_t color);

#endif // GRAPHICS_H
