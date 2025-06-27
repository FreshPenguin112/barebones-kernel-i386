#ifndef LIMINE_H
#define LIMINE_H
#include <stdint.h>
#include <stddef.h>

#define LIMINE_FRAMEBUFFER_REQUEST 0x3e7e279702be32af
#define LIMINE_FRAMEBUFFER_RESPONSE 0x3e7e279702be32af

struct limine_framebuffer {
    void *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
};

struct limine_framebuffer_request {
    uint64_t id;
    uint64_t revision;
    void *response;
};

struct limine_framebuffer_response {
    uint64_t id;
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer **framebuffers;
};

#endif // LIMINE_H
