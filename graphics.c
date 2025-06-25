#include "graphics.h"
#include "syscall.h"
#include <stdint.h>

void set_graphics_mode() {
    syscall(SYSCALL_SET_MODE, 0x13, 0, 0);
}

void set_text_mode() {
    syscall(SYSCALL_SET_MODE, 0x03, 0, 0);
}

void put_pixel(int x, int y, uint8_t color) {
    syscall(SYSCALL_PUT_PIXEL, x, y, color);
}

void draw_hline(int x, int y, int length, uint8_t color) {
    for (int i = 0; i < length; i++)
        put_pixel(x + i, y, color);
}

void draw_vline(int x, int y, int length, uint8_t color) {
    for (int i = 0; i < length; i++)
        put_pixel(x, y + i, color);
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y + h - 1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x + w - 1, y, h, color);
}

void fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++)
        draw_hline(x, y + j, w, color);
}

void draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sy = (y0 < y1) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    while (1) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void draw_circle(int xc, int yc, int r, uint8_t color) {
    int x = 0, y = r, d = 3 - 2 * r;
    while (y >= x) {
        put_pixel(xc + x, yc + y, color);
        put_pixel(xc - x, yc + y, color);
        put_pixel(xc + x, yc - y, color);
        put_pixel(xc - x, yc - y, color);
        put_pixel(xc + y, yc + x, color);
        put_pixel(xc - y, yc + x, color);
        put_pixel(xc + y, yc - x, color);
        put_pixel(xc - y, yc - x, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void fill_circle(int xc, int yc, int r, uint8_t color) {
    int x = 0, y = r, d = 3 - 2 * r;
    while (y >= x) {
        draw_hline(xc - x, yc - y, 2 * x + 1, color);
        draw_hline(xc - x, yc + y, 2 * x + 1, color);
        draw_hline(xc - y, yc - x, 2 * y + 1, color);
        draw_hline(xc - y, yc + x, 2 * y + 1, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void clear_screen(uint8_t color) {
    fill_rect(0, 0, VGA_GFX_WIDTH, VGA_GFX_HEIGHT, color);
}
