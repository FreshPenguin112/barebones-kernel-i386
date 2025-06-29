#pragma once
#include <stdint.h>

struct limine_framebuffer_request {
    uint64_t id[4];
    uint64_t revision;
    void *response;
} __attribute__((packed));

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer **framebuffers;
} __attribute__((packed));

struct limine_framebuffer {
    void *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t reserved[7];
} __attribute__((packed));
