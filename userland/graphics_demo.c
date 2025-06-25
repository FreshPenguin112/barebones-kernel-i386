#include "../graphics.h"
#include "../syscall.h"
#include <stdint.h>

// Simple color palette for demo
static const uint8_t colors[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
#define NUM_COLORS (sizeof(colors)/sizeof(colors[0]))



void _start() {
    // Set graphics mode (mode 0x13 = 320x200x256)
    syscall(SYSCALL_SET_MODE, 0x13);

    // Clear screen to black
    clear_screen(0);

    // Draw color bars
    for (int i = 0; i < NUM_COLORS; i++) {
        fill_rect(i * 20, 10, 18, 40, colors[i]);
    }

    // Draw rectangles
    for (int i = 0; i < 5; i++) {
        draw_rect(30 + i*30, 70 + i*10, 40, 30, 12 + i);
    }

    // Draw lines
    for (int i = 0; i < 10; i++) {
        draw_line(0, 100 + i*10, 319, 199 - i*10, 8 + i);
    }

    // Draw circles
    for (int r = 10; r < 80; r += 15) {
        draw_circle(160, 100, r, 14);
    }
    fill_circle(160, 100, 30, 3);

    // Wait (simple loop)
    for (volatile int t = 0; t < 20000000; t++);

    // Return to text mode (mode 0x03)
    syscall(SYSCALL_SET_MODE, 0x03);

    syscall(SYSCALL_EXIT);
}
