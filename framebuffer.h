#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <stdint.h>
#include "multiboot.h"
#include "limine.h"

// Framebuffer info struct
struct framebuffer_info {
    uint32_t address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
};

extern struct framebuffer_info fb_info;

// Try to detect and initialize framebuffer. Returns 1 if found, 0 otherwise.
int framebuffer_init(void);
// Try to detect and initialize framebuffer from Multiboot info. Returns 1 if found, 0 otherwise.
int framebuffer_init_multiboot(struct multiboot_info *mbi);
// Try to detect and initialize framebuffer from Limine. Returns 1 if found, 0 otherwise.
int framebuffer_init_limine(void);
// Plot a pixel at (x, y) with RGB color
void framebuffer_plot_pixel(int x, int y, uint32_t rgb);
// Clear the framebuffer with a color
void framebuffer_clear(uint32_t rgb);

#endif // FRAMEBUFFER_H
