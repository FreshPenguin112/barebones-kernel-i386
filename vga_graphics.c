#include "vga_graphics.h"
#include "framebuffer.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_ADDRESS 0xA0000

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static int use_framebuffer = 0;

extern void kernel_print(const char *text);

// Set VGA mode 13h (320x200x256) in protected mode
void vga_set_mode(void) {
    use_framebuffer = framebuffer_init_limine();
    if (!use_framebuffer) {
        use_framebuffer = framebuffer_init_limine();
    }
    kernel_print("VGA mode: ");
    if (use_framebuffer) {
        kernel_print("Framebuffer\n");
        return;
    }
    kernel_print("Text mode\n");

    // Set Miscellaneous Output Register
    outb(0x3C2, 0x63);

    // Sequencer Registers
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);
    outb(0x3C4, 0x01); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F);
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E);

    // Unlock CRTC registers
    outb(0x3D4, 0x11); outb(0x3D5, 0x00);

    // CRTC Registers (mode 13h)
    static const uint8_t crtc[] = {
        0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
        0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
        0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,
        0xFF
    };
    for (int i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }

    // Graphics Controller
    outb(0x3CE, 0x00); outb(0x3CF, 0x00);
    outb(0x3CE, 0x01); outb(0x3CF, 0x00);
    outb(0x3CE, 0x02); outb(0x3CF, 0x00);
    outb(0x3CE, 0x03); outb(0x3CF, 0x00);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x40);
    outb(0x3CE, 0x06); outb(0x3CF, 0x05);
    outb(0x3CE, 0x07); outb(0x3CF, 0x0F);
    outb(0x3CE, 0x08); outb(0x3CF, 0xFF);

    // Attribute Controller
    for (int i = 0; i < 16; i++) {
        outb(0x3C0, i);
        outb(0x3C0, i);
    }
    for (int i = 16; i < 32; i++) {
        outb(0x3C0, i);
        outb(0x3C0, 0);
    }
    outb(0x3C0, 0x20); // Enable video

    // Clear framebuffer to black
    uint8_t *vga = (uint8_t *)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        vga[i] = 0;
    }
}

// Restore text mode (mode 3) in protected mode
void vga_text_mode(void) {
    // Miscellaneous Output Register
    outb(0x3C2, 0x67);
    // Sequencer Registers
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);
    outb(0x3C4, 0x01); outb(0x3C5, 0x00);
    outb(0x3C4, 0x02); outb(0x3C5, 0x03);
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);
    outb(0x3C4, 0x04); outb(0x3C5, 0x02);
    // CRTC Registers (unlock first)
    outb(0x3D4, 0x11); outb(0x3D5, 0x0E);
    // CRTC
    static const uint8_t crtc[] = {
        0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,
        0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x50,
        0x9C,0x0E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
        0xFF
    };
    for (int i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }
    // Graphics Controller
    outb(0x3CE, 0x00); outb(0x3CF, 0x00);
    outb(0x3CE, 0x01); outb(0x3CF, 0x00);
    outb(0x3CE, 0x02); outb(0x3CF, 0x00);
    outb(0x3CE, 0x03); outb(0x3CF, 0x00);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);
    outb(0x3CE, 0x07); outb(0x3CF, 0x00);
    outb(0x3CE, 0x08); outb(0x3CF, 0xFF);
    // Attribute Controller
    for (int i = 0; i < 16; i++) {
        outb(0x3C0, i);
        outb(0x3C0, i);
    }
    for (int i = 16; i < 32; i++) {
        outb(0x3C0, i);
        outb(0x3C0, 0);
    }
    outb(0x3C0, 0x20); // Enable video
}

void vga_plot_pixel(int x, int y, uint8_t color) {
    if (use_framebuffer) {
        // Map VGA color index to RGB (simple 6-bit mapping)
        static const uint32_t vga_palette[16] = {
            0x000000,0x0000AA,0x00AA00,0x00AAAA,0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
            0x555555,0x5555FF,0x55FF55,0x55FFFF,0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF
        };
        framebuffer_plot_pixel(x, y, vga_palette[color & 0x0F]);
        return;
    }
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    uint8_t *vga = (uint8_t *)VGA_ADDRESS;
    vga[y * VGA_WIDTH + x] = color;
}

void vga_plot_pixel_rgb(int x, int y, uint32_t rgb) {
    if (use_framebuffer) {
        framebuffer_plot_pixel(x, y, rgb);
    }
}

void vga_clear(uint8_t color) {
    if (use_framebuffer) {
        static const uint32_t vga_palette[16] = {
            0x000000,0x0000AA,0x00AA00,0x00AAAA,0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
            0x555555,0x5555FF,0x55FF55,0x55FFFF,0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF
        };
        framebuffer_clear(vga_palette[color & 0x0F]);
        return;
    }
    // Clear framebuffer to black
    uint8_t *vga = (uint8_t *)0xA0000;
    for (int i = 0; i < 320 * 200; i++) {
        vga[i] = 0;
    }
}

void vga_clear_rgb(uint32_t rgb) {
    if (use_framebuffer) {
        framebuffer_clear(rgb);
    }
}