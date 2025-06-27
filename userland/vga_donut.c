#include "../syscall.h"
#include "../small_math.h"

#define WIDTH 320
#define HEIGHT 200
#define CX (WIDTH/2)
#define CY (HEIGHT/2)
#define R1 40.0f
#define R2 100.0f
#define K2 200.0f
#define K1 80.0f

// Simple palette: 0=black, 1=blue, 2=green, 3=cyan, 4=red, 5=magenta, 6=yellow, 7=white
static unsigned char colors[] = {0, 4, 6, 2, 3, 5, 1, 7};

extern float my_sinf(float x);
extern float my_cosf(float x);

void _start(void) {
    syscall(SYSCALL_VGA_SET_MODE);
    float A = 0, B = 0;
    float zbuf[WIDTH * HEIGHT];
    for (int frame = 0; frame < 200; frame++) {
        // Clear screen (black) and z-buffer
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++) {
                syscall(SYSCALL_VGA_PLOT_PIXEL, x, y, 0);
                zbuf[y * WIDTH + x] = -1e9f;
            }
        // Donut math
        for (float theta = 0; theta < 6.28f; theta += 0.07f) {
            for (float phi = 0; phi < 6.28f; phi += 0.02f) {
                float sinA = my_sinf(A), cosA = my_cosf(A);
                float sinB = my_sinf(B), cosB = my_cosf(B);
                float sintheta = my_sinf(theta), costheta = my_cosf(theta);
                float sinphi = my_sinf(phi), cosphi = my_cosf(phi);
                float circlex = R2 + R1 * costheta;
                float circley = R1 * sintheta;
                float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
                float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
                float z = K2 + cosA * circlex * sinphi + circley * sinA;
                float ooz = 1.0f / z;
                int xp = (int)(CX + K1 * ooz * x);
                int yp = (int)(CY - K1 * ooz * y);
                int luminance = (int)(8 * ((cosphi * costheta * sinB - cosA * costheta * sinphi - sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi))));
                if (luminance > 0 && xp >= 0 && xp < WIDTH && yp >= 0 && yp < HEIGHT) {
                    int idx = yp * WIDTH + xp;
                    if (ooz > zbuf[idx]) {
                        zbuf[idx] = ooz;
                        syscall(SYSCALL_VGA_PLOT_PIXEL, xp, yp, colors[luminance % 8]);
                    }
                }
                // DEBUG: plot all in-bounds points in white, ignore luminance and z-buffer
                if (xp >= 0 && xp < WIDTH && yp >= 0 && yp < HEIGHT) {
                    syscall(SYSCALL_VGA_PLOT_PIXEL, xp, yp, 7); // 7 = white
                }
            }
        }
        A += 0.04f;
        B += 0.02f;
        // crude delay
        for (volatile int d = 0; d < 1000000; d++);
    }
    syscall(SYSCALL_VGA_TEXT_MODE);
    syscall(SYSCALL_PRINT, "Donut demo done.\n");
    return;
}
