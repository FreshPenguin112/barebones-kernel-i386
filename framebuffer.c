#include "framebuffer.h"
#include "io.h"
#include <stdint.h>

// Bochs/QEMU VBE MMIO base address (default)
#define BGA_MMIO_BASE 0xE0000000
#define BGA_REG_ID    0x0
#define BGA_REG_XRES  0x1
#define BGA_REG_YRES  0x2
#define BGA_REG_BPP   0x3
#define BGA_REG_ENABLE 0x4
#define BGA_REG_FB    0x8

#define BGA_ID        0xB0C5

#define BGA_IOPORT_INDEX 0x01CE
#define BGA_IOPORT_DATA  0x01CF

static inline void outw(uint16_t port, uint16_t val);
static inline uint16_t inw(uint16_t port);

static inline uint16_t bga_read_reg_mmio(uint16_t reg) {
    volatile uint16_t *addr = (volatile uint16_t *)(BGA_MMIO_BASE + (reg << 1));
    return *addr;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t bga_read_reg_io(uint16_t reg) {
    outw(BGA_IOPORT_INDEX, reg);
    return inw(BGA_IOPORT_DATA);
}

uint16_t framebuffer_bga_read_reg(uint16_t reg) {
    // Try MMIO first
    uint16_t id = bga_read_reg_mmio(BGA_REG_ID);
    if (id == BGA_ID)
        return bga_read_reg_mmio(reg);
    // Fallback to I/O port
    id = bga_read_reg_io(BGA_REG_ID);
    if (id == BGA_ID)
        return bga_read_reg_io(reg);
    // Neither worked
    return 0;
}

static void framebuffer_bga_write_reg(uint16_t reg, uint16_t value) {
    // Try MMIO first
    if (bga_read_reg_mmio(BGA_REG_ID) == BGA_ID) {
        volatile uint16_t *addr = (volatile uint16_t *)(BGA_MMIO_BASE + (reg << 1));
        *addr = value;
        return;
    }
    // Fallback to I/O port
    outw(BGA_IOPORT_INDEX, reg);
    outw(BGA_IOPORT_DATA, value);
}

void framebuffer_set_mode(uint16_t w, uint16_t h, uint16_t bpp) {
    framebuffer_bga_write_reg(BGA_REG_ENABLE, 0); // Disable
    framebuffer_bga_write_reg(BGA_REG_XRES, w);
    framebuffer_bga_write_reg(BGA_REG_YRES, h);
    framebuffer_bga_write_reg(BGA_REG_BPP, bpp);
    framebuffer_bga_write_reg(BGA_REG_ENABLE, 0x41); // Enable + LFB
}

void framebuffer_detect(struct framebuffer_info *fb) {
    fb->found = 0;
    uint16_t id = framebuffer_bga_read_reg(BGA_REG_ID);
    if (id == BGA_ID) {
        fb->width = framebuffer_bga_read_reg(BGA_REG_XRES);
        fb->height = framebuffer_bga_read_reg(BGA_REG_YRES);
        fb->bpp = framebuffer_bga_read_reg(BGA_REG_BPP);
        fb->addr = (uint32_t *)(BGA_MMIO_BASE + 0x4000); // Default framebuffer offset
        fb->found = 1;
        // Pitch is usually width * (bpp/8)
        fb->pitch = fb->width * (fb->bpp / 8);
    }
}
