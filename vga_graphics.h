#ifndef VGA_GRAPHICS_H
#define VGA_GRAPHICS_H

#include <stdint.h>
#include "framebuffer.h"

// Set VGA mode 13h (320x200x256)
void vga_set_mode(void);
// Restore text mode (mode 3)
void vga_text_mode(void);
// Plot a pixel at (x, y) with color index (VGA) or RGB (framebuffer)
void vga_plot_pixel(int x, int y, uint8_t color);
void vga_plot_pixel_rgb(int x, int y, uint32_t rgb);
// Clear the screen/framebuffer with color
void vga_clear(uint8_t color);
void vga_clear_rgb(uint32_t rgb);

#endif // VGA_GRAPHICS_H
