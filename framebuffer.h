#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>

struct framebuffer_info {
    uint32_t *addr;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    int found;
};

void framebuffer_detect(struct framebuffer_info *fb);
uint16_t framebuffer_bga_read_reg(uint16_t reg);
void framebuffer_set_mode(uint16_t w, uint16_t h, uint16_t bpp);

#endif // FRAMEBUFFER_H
