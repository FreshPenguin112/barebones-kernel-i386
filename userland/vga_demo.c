#include "../syscall.h"

// Simple userland demo: set VGA mode, clear, draw a diagonal, then restore text mode
void _start(void) {
    syscall(SYSCALL_VGA_SET_MODE);
    // Draw a red diagonal from (0,0) to (199,199)
    for (int i = 0; i < 200; i++) {
        syscall(SYSCALL_VGA_PLOT_PIXEL, i, i, 4); // 4 = red in standard VGA palette
    }
    // Draw a green diagonal from (0,199) to (199,0)
    for (int i = 0; i < 200; i++) {
        syscall(SYSCALL_VGA_PLOT_PIXEL, i, 199 - i, 2); // 2 = green
    }
    // Draw a blue box border
    for (int x = 0; x < 320; x++) {
        syscall(SYSCALL_VGA_PLOT_PIXEL, x, 0, 1); // top
        syscall(SYSCALL_VGA_PLOT_PIXEL, x, 199, 1); // bottom
    }
    for (int y = 0; y < 200; y++) {
        syscall(SYSCALL_VGA_PLOT_PIXEL, 0, y, 1); // left
        syscall(SYSCALL_VGA_PLOT_PIXEL, 319, y, 1); // right
    }
    // Wait for a key press before restoring text mode
    char c;
    syscall(SYSCALL_GETCHAR, &c);
    syscall(SYSCALL_VGA_TEXT_MODE);
    syscall(SYSCALL_PRINT, "VGA demo done.\n");
    return;
}
