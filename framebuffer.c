#include "framebuffer.h"
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

// Limine framebuffer request (must be global, not static, and in .limine_requests)
__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

struct framebuffer_info fb_info = {0};

// Limine framebuffer init
int framebuffer_init_limine(void) {
    struct limine_framebuffer_response *resp = (struct limine_framebuffer_response *)framebuffer_request.response;
    if (!resp || resp->framebuffer_count < 1) return 0;
    struct limine_framebuffer *fb = resp->framebuffers[0];
    fb_info.address = (uint64_t)fb->address;
    fb_info.width = fb->width;
    fb_info.height = fb->height;
    fb_info.pitch = fb->pitch;
    fb_info.bpp = fb->bpp;
    return 1;
}

void framebuffer_plot_pixel(int x, int y, uint32_t rgb) {
    if (x < 0 || y < 0 || x >= (int)fb_info.width || y >= (int)fb_info.height) return;
    uint32_t *fb = (uint32_t *)(uintptr_t)fb_info.address;
    fb[y * (fb_info.pitch / 4) + x] = rgb;
}

void framebuffer_clear(uint32_t rgb) {
    uint32_t *fb = (uint32_t *)(uintptr_t)fb_info.address;
    for (uint32_t y = 0; y < fb_info.height; y++) {
        for (uint32_t x = 0; x < fb_info.width; x++) {
            fb[y * (fb_info.pitch / 4) + x] = rgb;
        }
    }
}
