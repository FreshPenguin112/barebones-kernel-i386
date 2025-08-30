// Global flag to exit window manager
volatile int wm_should_exit = 0;
#include <stddef.h>
#include <stdint.h>
#include "shell.h" // Change to use quotes instead of angle brackets
#include "serial.h"
#include "qemu_utils.h"
#include "tarfs.h"
#include "syscall.h"
#include "idt.h"
#include "string_utils.h"
#include "pit.h"
#include "io.h"
#include "framebuffer.h"
#include "font8x8_basic.h"

#define MAX_WINDOWS 8
#define WINDOW_TITLE_HEIGHT 24
#define WINDOW_BORDER 2
#define WINDOW_MIN_WIDTH 120
#define WINDOW_MIN_HEIGHT 60
#define CLOSE_BTN_SIZE 16

// -----------------------------------------------------------------------------
// Output capture for piping
static int output_capture_enabled = 0;
static char *output_capture_buffer = 0;
static size_t output_capture_bufsize = 0;
static size_t output_capture_pos = 0;

void kernel_output_capture_start(char *buf, size_t bufsize) {
    output_capture_enabled = 1;
    output_capture_buffer = buf;
    output_capture_bufsize = bufsize;
    output_capture_pos = 0;
    if (output_capture_buffer && output_capture_bufsize > 0)
        output_capture_buffer[0] = 0;
}

void kernel_output_capture_stop(void) {
    output_capture_enabled = 0;
    output_capture_buffer = 0;
    output_capture_bufsize = 0;
    output_capture_pos = 0;
}

// VGA text‐mode state
// -----------------------------------------------------------------------------
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F; // white on black

volatile int user_program_exited = 0;
volatile uint32_t timer_ticks = 0;

// -----------------------------------------------------------------------------
// Serial (COM1) setup
// -----------------------------------------------------------------------------
// #define COM1_PORT 0x3F8

/*
void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB
    outb(COM1_PORT + 0, 0x03);    // Divisor low = 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00);    // Divisor high
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, 1 stop bit
    outb(COM1_PORT + 2, 0xC7);    // FIFO enabled, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    while (!serial_is_transmit_empty()) {}
    outb(COM1_PORT, c);
}
*/


// -----------------------------------------------------------------------------
// VGA text‐mode routines
// -----------------------------------------------------------------------------
void term_init(void)
{
    for (int r = 0; r < VGA_ROWS; r++)
    {
        for (int c = 0; c < VGA_COLS; c++)
        {
            vga_buffer[r * VGA_COLS + c] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_row = term_col = 0;
}

void term_setcolor(uint8_t fg, uint8_t bg)
{
    term_color = (bg << 4) | (fg & 0x0F);
}

void term_reset_color(void)
{
    term_setcolor(0x0F, 0x00);
}

uint8_t ansi_to_vga_color(int code)
{
    // maps 30-37 or 40-47 → VGA palette
    switch (code)
    {
    case 30:
        return 0;
    case 31:
        return 4;
    case 32:
        return 2;
    case 33:
        return 6;
    case 34:
        return 1;
    case 35:
        return 5;
    case 36:
        return 3;
    case 37:
        return 7;
    default:
        return 7;
    }
}

void term_handle_ansi_code(int code)
{
    if (code == 0)
    {
        term_reset_color();
    }
    else if (code >= 30 && code <= 37)
    {
        // set fg only
        term_setcolor(ansi_to_vga_color(code), term_color >> 4);
    }
    else if (code >= 40 && code <= 47)
    {
        // set bg only
        term_setcolor(term_color & 0x0F, ansi_to_vga_color(code - 10));
    }
}

void term_putc_vga(char c)
{
    if (c == '\n')
    {
        term_col = 0;
        term_row++;
    }
    else
    {
        vga_buffer[term_row * VGA_COLS + term_col] =
            ((uint16_t)term_color << 8) | c;
        term_col++;
    }
    if (term_col >= VGA_COLS)
    {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS)
    {
        term_row = 0;
    }
}

// -----------------------------------------------------------------------------
// Unified “putc” for serial + VGA
// -----------------------------------------------------------------------------
void kernel_putc(char c)
{
    if (output_capture_enabled && output_capture_buffer && output_capture_pos < output_capture_bufsize - 1) {
        output_capture_buffer[output_capture_pos++] = c;
        output_capture_buffer[output_capture_pos] = 0;
    } else {
        serial_write_char(c);
        term_putc_vga(c);
    }
}

// send ESC [ … m to serial & update VGA; `seq` is digits only, no ‘m’
void kernel_handle_ansi_and_putc(const char *seq, size_t len)
{
    serial_write_str("\033[");
    for (size_t i = 0; i < len; i++)
        serial_write_char(seq[i]);
    serial_write_char('m');

    // parse the integer
    int code = 0;
    for (size_t i = 0; i < len; i++)
        code = code * 10 + (seq[i] - '0');
    term_handle_ansi_code(code);
}

// print a plain C string, handling only “ESC[…m” sequences
void kernel_print(const char *s)
{
    for (size_t i = 0; s[i]; i++)
    {
        if (s[i] == '\033' && s[i + 1] == '[')
        {
            size_t j = i + 2;
            while (s[j] && s[j] != 'm')
                j++;
            if (s[j] == 'm')
            {
                if (!output_capture_enabled)
                    kernel_handle_ansi_and_putc(&s[i + 2], j - (i + 2));
                i = j;
                continue;
            }
        }
        kernel_putc(s[i]);
    }
}

// -----------------------------------------------------------------------------
// Map english names → ANSI codes
// -----------------------------------------------------------------------------
static int ansi_fg_code(const char *c)
{
    if (!strcmp(c, "black"))
        return 30;
    else if (!strcmp(c, "red"))
        return 31;
    else if (!strcmp(c, "green"))
        return 32;
    else if (!strcmp(c, "yellow"))
        return 33;
    else if (!strcmp(c, "blue"))
        return 34;
    else if (!strcmp(c, "magenta"))
        return 35;
    else if (!strcmp(c, "cyan"))
        return 36;
    else if (!strcmp(c, "white"))
        return 37;
    else
        return -1;
}
static int ansi_bg_code(const char *c)
{
    if (!strcmp(c, "black"))
        return 40;
    else if (!strcmp(c, "red"))
        return 41;
    else if (!strcmp(c, "green"))
        return 42;
    else if (!strcmp(c, "yellow"))
        return 43;
    else if (!strcmp(c, "blue"))
        return 44;
    else if (!strcmp(c, "magenta"))
        return 45;
    else if (!strcmp(c, "cyan"))
        return 46;
    else if (!strcmp(c, "white"))
        return 47;
    else
        return -1;
}

// format a two‐digit code into `out[]`, return length (always 2 here)
static size_t fmt_code(char out[2], int code)
{
    out[0] = '0' + (code / 10);
    out[1] = '0' + (code % 10);
    return 2;
}

// -----------------------------------------------------------------------------
// High-level ANSI wrapper
// -----------------------------------------------------------------------------
void kernel_print_ansi(const char *text,
                       const char *fg_name,
                       const char *bg_name)
{
    char buf[2];
    size_t len;
    int fg = ansi_fg_code(fg_name);
    int bg = ansi_bg_code(bg_name);

    if (!output_capture_enabled) {
        if (fg >= 0)
        {
            len = fmt_code(buf, fg);
            kernel_handle_ansi_and_putc(buf, len);
        }
        if (bg >= 0)
        {
            len = fmt_code(buf, bg);
            kernel_handle_ansi_and_putc(buf, len);
        }
    }
    // now the text
    for (const char *p = text; *p; p++)
        kernel_putc(*p);
    // reset if we changed anything
    if ((fg >= 0 || bg >= 0) && !output_capture_enabled)
        kernel_handle_ansi_and_putc("0", 1);
}

void kernel_putc_ansi(const char text,
                       const char *fg_name,
                       const char *bg_name)
{
    char buf[2];
    size_t len;
    int fg = ansi_fg_code(fg_name);
    int bg = ansi_bg_code(bg_name);

    if (!output_capture_enabled) {
        if (fg >= 0)
        {
            len = fmt_code(buf, fg);
            kernel_handle_ansi_and_putc(buf, len);
        }
        if (bg >= 0)
        {
            len = fmt_code(buf, bg);
            kernel_handle_ansi_and_putc(buf, len);
        }
    }
    // now the text
    kernel_putc(text);
    // reset if we changed anything
    if ((fg >= 0 || bg >= 0) && !output_capture_enabled)
        kernel_handle_ansi_and_putc("0", 1);
}

// -----------------------------------------------------------------------------
// ANSI screen/cursor controls
// -----------------------------------------------------------------------------
void ansi_clear(void)
{
    serial_write_str("\033[2J");
    term_init();
}
void ansi_home(void)
{
    serial_write_str("\033[H");
    term_row = term_col = 0;
}
void ansi_clearhome(void)
{
    serial_write_str("\033[3J\033[H\033[2J");
}

// -----------------------------------------------------------------------------
// Syscall handler
// -----------------------------------------------------------------------------
void (*elf_exit_callback)(void) = 0;

extern void syscall_handler_asm();

void syscall_handler(int syscall_number, void *arg1, void *arg2, void *arg3)
{
    syscall_dispatcher(syscall_number, arg1, arg2, arg3);
}

// Set up the syscall interrupt (int 0x80)
void setup_syscalls()
{
    idt_set_gate(0x80, (uint32_t)syscall_handler_asm, 0x08, 0x8E);
}

// -----------------------------------------------------------------------------
// Timer handler
// -----------------------------------------------------------------------------
extern void timer_handler_asm(void);

void timer_handler(void)
{
    timer_ticks++;
    outb(0x20, 0x20);
}

// Wait for a given number of milliseconds using PIT timer_ticks
static void wait_milliseconds(uint32_t ms) {
    uint32_t start = timer_ticks;
    while ((timer_ticks - start) < ms) {
        asm volatile("hlt");
    }
}

// -----------------------------------------------------------------------------
// PS/2 Keyboard and Mouse support (basic)
// -----------------------------------------------------------------------------
#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_CMD_PORT 0x64

// Wait for PS/2 controller input buffer to be clear
static void ps2_wait_input_clear(void) {
    for (int i = 0; i < 10000; i++) {
        if (!(inb(PS2_STATUS_PORT) & 0x02)) return;
    }
}
// Wait for PS/2 controller output buffer to be full
static void ps2_wait_output_full(void) {
    for (int i = 0; i < 10000; i++) {
        if (inb(PS2_STATUS_PORT) & 0x01) return;
    }
}
// Send a command to the PS/2 keyboard and wait for ACK
static void ps2_keyboard_send(uint8_t cmd) {
    ps2_wait_input_clear();
    outb(PS2_DATA_PORT, cmd);
    ps2_wait_output_full();
    uint8_t resp = inb(PS2_DATA_PORT);
    if (resp != 0xFA) {
        // Optionally: handle resend (0xFE) or error
    }
}



// PS/2 controller init: enable devices and interrupts
void ps2_init(void) {
    // Clear output buffer
    while (inb(PS2_STATUS_PORT) & 0x01) {
        (void)inb(PS2_DATA_PORT);
    }
    // Enable keyboard (first PS/2 port)
    outb(PS2_CMD_PORT, 0xAE); // Enable keyboard
    // Enable mouse (second PS/2 port)
    outb(PS2_CMD_PORT, 0xA8); // Enable mouse
    // Set config byte: enable IRQ1 and IRQ12
    ps2_wait_input_clear();
    outb(PS2_CMD_PORT, 0x20); // Read config byte
    ps2_wait_output_full();
    uint8_t config = inb(PS2_DATA_PORT);
    config |= 0x03; // Enable IRQ1 and IRQ12
    ps2_wait_input_clear();
    outb(PS2_CMD_PORT, 0x60); // Write config byte
    ps2_wait_input_clear();
    outb(PS2_DATA_PORT, config);
    // Enable mouse device
    outb(PS2_CMD_PORT, 0xD4);
    outb(PS2_DATA_PORT, 0xF4); // Enable data reporting
    // Enable keyboard scanning
    ps2_keyboard_send(0xF4); // Enable scanning
}

// -----------------------------------------------------------------------------
// PIC initialization and remapping
// -----------------------------------------------------------------------------
static void pic_remap(void) {
    // Start initialization
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // Set vector offset
    outb(0x21, 0x20); // Master PIC: 0x20 (32)
    outb(0xA1, 0x28); // Slave PIC: 0x28 (40)
    // Tell Master PIC there is a slave at IRQ2 (0000 0100)
    outb(0x21, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(0xA1, 0x02);
    // Set 8086/88 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // Unmask IRQ0 (timer) and IRQ1 (keyboard) only
    outb(0x21, 0xFC); // 1111 1100: IRQ0 and IRQ1 enabled
    outb(0xA1, 0xEF); // 1110 1111: only IRQ12 enabled
}

extern void keyboard_handler_asm(void);
extern void mouse_handler_asm(void);


// -----------------------------------------------------------------------------
// Window manager (remade, minimal, clean)
// Function declarations for use in other translation units
// -----------------------------------------------------------------------------

// --- Minimal, clean window manager implementation ---
#define WM_MAX_WINDOWS 10000
#define WM_TITLE_HEIGHT 24
#define WM_BORDER 2
#define WM_MIN_WIDTH 120
#define WM_MIN_HEIGHT 60
#define WM_CLOSE_SIZE 16

struct WM_Window {
    int x, y, w, h;
    int dragging;
    int drag_dx, drag_dy;
    int alive;
};
static struct WM_Window WM_windows[WM_MAX_WINDOWS];
static int WM_window_count = 0;
static int WM_mouse_x = 0, WM_mouse_y = 0;
static int WM_mouse_left = 0, WM_mouse_left_prev = 0;
static char WM_key = 0;

// Rectangle helpers (move these above WM_draw_window)
static void WM_fill_rect(uint32_t *fb, int fbw, int fbh, int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        int py = y + j;
        if (py < 0 || py >= fbh) continue;
        for (int i = 0; i < w; i++) {
            int px = x + i;
            if (px < 0 || px >= fbw) continue;
            fb[py * fbw + px] = color | 0xFF000000;
        }
    }
}
static void WM_draw_rect(uint32_t *fb, int fbw, int fbh, int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < w; i++) {
        if (y >= 0 && y < fbh && x + i >= 0 && x + i < fbw)
            fb[y * fbw + (x + i)] = color | 0xFF000000;
        if (y + h - 1 >= 0 && y + h - 1 < fbh && x + i >= 0 && x + i < fbw)
            fb[(y + h - 1) * fbw + (x + i)] = color | 0xFF000000;
    }
    for (int j = 0; j < h; j++) {
        if (x >= 0 && x < fbw && y + j >= 0 && y + j < fbh)
            fb[(y + j) * fbw + x] = color | 0xFF000000;
        if (x + w - 1 >= 0 && x + w - 1 < fbw && y + j >= 0 && y + j < fbh)
            fb[(y + j) * fbw + (x + w - 1)] = color | 0xFF000000;
    }
}

static void WM_draw_char(uint32_t *fb, int fbw, int fbh, int x, int y, char c, uint32_t color) {
    if (c < 0 || c > 127) return;
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if ((bits >> col) & 1) {
                int px = x + col * 2, py = y + row * 2;
                for (int dy = 0; dy < 2; dy++) {
                    for (int dx = 0; dx < 2; dx++) {
                        int fx = px + dx, fy = py + dy;
                        if (fx >= 0 && fx < fbw && fy >= 0 && fy < fbh)
                            fb[fy * fbw + fx] = color | 0xFF000000;
                    }
                }
            }
        }
    }
}
// Draw text at double size
static void WM_draw_text(uint32_t *fb, int fbw, int fbh, int x, int y, const char *s, uint32_t color) {
    for (int i = 0; s[i]; i++)
        WM_draw_char(fb, fbw, fbh, x + i * 16, y, s[i], color);
}
// Make the mouse cursor much larger and bolder
static void WM_draw_mouse(uint32_t *fb, int fbw, int fbh, int mx, int my) {
    // White outline crosshair (thicker)
    for (int i = -15; i <= 15; i++) {
        for (int t = -2; t <= 2; t++) {
            int px = mx + i, py = my + t;
            if (px >= 0 && px < fbw && py >= 0 && py < fbh)
                fb[py * fbw + px] = 0xFFFFFFFF;
            px = mx + t, py = my + i;
            if (px >= 0 && px < fbw && py >= 0 && py < fbh)
                fb[py * fbw + px] = 0xFFFFFFFF;
        }
    }
    // Black crosshair (inside white)
    for (int i = -11; i <= 11; i++) {
        for (int t = -1; t <= 1; t++) {
            int px = mx + i, py = my + t;
            if (px >= 0 && px < fbw && py >= 0 && py < fbh)
                fb[py * fbw + px] = 0x000000;
            px = mx + t, py = my + i;
            if (px >= 0 && px < fbw && py >= 0 && py < fbh)
                fb[py * fbw + px] = 0x000000;
        }
    }
    // Large white center square
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int px = mx + dx, py = my + dy;
            if (px >= 0 && px < fbw && py >= 0 && py < fbh)
                fb[py * fbw + px] = 0xFFFFFFFF;
        }
    }
}
// Draw the close button (move above WM_draw_window)
static void WM_draw_close(uint32_t *fb, int fbw, int fbh, int x, int y) {
    WM_fill_rect(fb, fbw, fbh, x, y, WM_CLOSE_SIZE, WM_CLOSE_SIZE, 0xCC2222);
    for (int i = 3; i < WM_CLOSE_SIZE - 3; i++) {
        int px1 = x + i, py1 = y + i;
        int px2 = x + WM_CLOSE_SIZE - 1 - i, py2 = y + i;
        if (px1 >= x && px1 < x + WM_CLOSE_SIZE && py1 >= y && py1 < y + WM_CLOSE_SIZE)
            fb[py1 * fbw + px1] = 0xFFFFFF;
        if (px2 >= x && px2 < x + WM_CLOSE_SIZE && py2 >= y && py2 < y + WM_CLOSE_SIZE)
            fb[py2 * fbw + px2] = 0xFFFFFF;
    }
}
static void WM_draw_window(uint32_t *fb, int fbw, int fbh, struct WM_Window *win, int focused) {
    // Draw a highly visible yellow window background
    WM_fill_rect(fb, fbw, fbh, win->x, win->y, win->w, win->h, 0xFFFFFF);
    // Draw a black border
    WM_draw_rect(fb, fbw, fbh, win->x, win->y, win->w, win->h, 0x000000);
    // Draw window title text
    WM_draw_text(fb, fbw, fbh, win->x + 8, win->y + 4, "Hello, World!", 0x000000);
    // Draw a magenta line under the title text to indicate draggable area
    int line_y = win->y + 24; // Just below the text (text is at y+4, double size = 16px, so y+20~y+24)
    int line_x0 = win->x + 8;
    int line_x1 = win->x + win->w - 8;
    if (line_x1 > line_x0) {
        for (int lx = line_x0; lx < line_x1; lx++) {
            for (int t = 0; t < 2; t++) { // 2px thick
                int ly = line_y + t;
                if (lx >= 0 && lx < fbw && ly >= 0 && ly < fbh)
                    fb[ly * fbw + lx] = 0x000000; // Black line
            }
        }
    }
    // Draw close button
    WM_draw_close(fb, fbw, fbh, win->x + win->w - WM_CLOSE_SIZE - 4, win->y + 4);
}
static int WM_hit_title(struct WM_Window *win, int mx, int my) {
    return mx >= win->x && mx < win->x + win->w && my >= win->y && my < win->y + WM_TITLE_HEIGHT;
}
static int WM_hit_close(struct WM_Window *win, int mx, int my) {
    int bx = win->x + win->w - WM_CLOSE_SIZE - 4;
    int by = win->y + 4;
    return mx >= bx && mx < bx + WM_CLOSE_SIZE && my >= by && my < by + WM_CLOSE_SIZE;
}
static void WM_bring_front(int idx) {
    if (idx < 0 || idx >= WM_window_count) return;
    struct WM_Window tmp = WM_windows[idx];
    for (int i = idx; i < WM_window_count - 1; i++)
        WM_windows[i] = WM_windows[i + 1];
    WM_windows[WM_window_count - 1] = tmp;
}
static void WM_spawn(void) {
    if (WM_window_count >= WM_MAX_WINDOWS) return;
    int x = 60 + (WM_window_count * 30) % 400;
    int y = 60 + (WM_window_count * 30) % 200;
    int w = WM_MIN_WIDTH + 120;
    int h = WM_MIN_HEIGHT + 40;
    // Correct struct initialization: x, y, w, h, dragging, drag_dx, drag_dy, alive
    WM_windows[WM_window_count++] = (struct WM_Window){x, y, w, h, 0, 0, 0, 1};
}
static void WM_remove_closed(void) {
    int j = 0;
    for (int i = 0; i < WM_window_count; i++) {
        if (WM_windows[i].alive)
            WM_windows[j++] = WM_windows[i];
    }
    WM_window_count = j;
}
void wm_update_key(char c) { WM_key = c; }
void wm_clear_keys(void) { WM_key = 0; }
void wm_update_mouse(int x, int y, int left) {
    WM_mouse_x = x; WM_mouse_y = y;
    WM_mouse_left_prev = WM_mouse_left;
    WM_mouse_left = left;
}
void window_manager_mainloop(void) {
    extern struct framebuffer_info fb;
    uint32_t *fb_ptr = fb.addr;
    int fbw = (fb.width > 0) ? fb.width : 800;
    int fbh = (fb.height > 0) ? fb.height : 600;
    // Allocate a mask buffer (1 byte per pixel)
    static uint8_t mask_static[1920 * 1080]; // Support up to 1920x1080, adjust as needed
    uint8_t *mask = mask_static;

    char wbuf[12];
    char hbuf[12];

    // --- Main loop ---
    int prev_mouse_x = -1, prev_mouse_y = -1;
    int prev_window_count = -1;
    unsigned int prev_window_hash = 0;
    while (!wm_should_exit) {
        // Compute window state hash
        unsigned int window_hash = WM_window_count;
        for (int i = 0; i < WM_window_count; i++) {
            window_hash ^= (WM_windows[i].x * 31 + WM_windows[i].y * 37 + WM_windows[i].w * 41 + WM_windows[i].h * 43 + WM_windows[i].alive * 47);
        }
        int redraw = 0;
        if (WM_mouse_x != prev_mouse_x || WM_mouse_y != prev_mouse_y) redraw = 1;
        if (WM_window_count != prev_window_count) redraw = 1;
        if (window_hash != prev_window_hash) redraw = 1;
        // Only redraw if something changed
        if (redraw) {
            // 1. Clear mask to 0
            for (int i = 0; i < fbw * fbh; i++) mask[i] = 0;
            // 2. Draw windows and set mask for covered pixels
            int focused = WM_window_count - 1;
            for (int i = 0; i < WM_window_count; i++) {
                if (WM_windows[i].alive) {
                    struct WM_Window *win = &WM_windows[i];
                    WM_draw_window(fb_ptr, fbw, fbh, win, i == focused);
                    int wx0 = win->x, wy0 = win->y, wx1 = win->x + win->w, wy1 = win->y + win->h;
                    if (wx0 < 0) wx0 = 0;
                    if (wy0 < 0) wy0 = 0;
                    if (wx1 > fbw) wx1 = fbw;
                    if (wy1 > fbh) wy1 = fbh;
                    for (int y = wy0; y < wy1; y++) {
                        for (int x = wx0; x < wx1; x++) {
                            mask[y * fbw + x] = 1;
                        }
                    }
                }
            }
            // 3. Erase only uncovered regions (mask==0)
            for (int i = 0; i < fbw * fbh; i++) {
                if (mask[i] == 0) fb_ptr[i] = 0xFF222222; // BGRA, force alpha
            }
            // 4. Draw mouse cursor on top
            WM_draw_mouse(fb_ptr, fbw, fbh, WM_mouse_x, WM_mouse_y);
        }
        // Save previous state
        prev_mouse_x = WM_mouse_x;
        prev_mouse_y = WM_mouse_y;
        prev_window_count = WM_window_count;
        prev_window_hash = window_hash;

        // Only spawn on key press transition (debounce)
        static char prev_key = 0;
        if (WM_key == 'n' && prev_key != 'n') { WM_spawn(); }
        if (WM_key == 'x' && prev_key != 'x') {
            for (int i = 0; i < WM_window_count; i++) {
                WM_windows[i].alive = 0; // Close all windows
            }
        }
        // Spawn as many windows as possible with 'a' key
        if (WM_key == 'a' && prev_key != 'a') {
            int to_spawn = WM_MAX_WINDOWS - WM_window_count;
            for (int i = 0; i < to_spawn; i++) {
                WM_spawn();
            }
        }
        prev_key = WM_key;
        wm_clear_keys();
        for (int i = WM_window_count - 1; i >= 0; i--) {
            struct WM_Window *win = &WM_windows[i];
            if (!win->alive) continue;
            if (WM_mouse_left && !WM_mouse_left_prev && WM_hit_close(win, WM_mouse_x, WM_mouse_y)) {
                win->alive = 0;
                break;
            }
            if (WM_mouse_left && !WM_mouse_left_prev && WM_hit_title(win, WM_mouse_x, WM_mouse_y)) {
                win->dragging = 1;
                win->drag_dx = WM_mouse_x - win->x;
                win->drag_dy = WM_mouse_y - win->y;
                WM_bring_front(i);
                break;
            }
            if (!WM_mouse_left && win->dragging) {
                win->dragging = 0;
            }
            if (win->dragging && WM_mouse_left) {
                win->x = WM_mouse_x - win->drag_dx;
                win->y = WM_mouse_y - win->drag_dy;
                if (win->x < 0) win->x = 0;
                if (win->y < 0) win->y = 0;
                if (win->x + win->w > fbw) win->x = fbw - win->w;
                if (win->y + win->h > fbh) win->y = fbh - win->h;
            }
        }
        WM_remove_closed();
        if (WM_window_count == 0) {
            WM_spawn();
        }
        shell_run_step(); // Run shell step to handle commands
        wait_milliseconds(10);
    }
}

// Patch keyboard/mouse handlers to update window manager state
// (Call these from the respective handlers)
// In keyboard_handler, after determining 'c':
//     if (c && c < 128 && c > 32) {
//         wm_update_key(c);
//     }
// In mouse_handler, after updating mouse_x, mouse_y:
//     int left = (flags & 0x01) ? 1 : 0;
//     wm_update_mouse(mouse_x, mouse_y, left);
// At end of keyboard_handler, clear keys:
//     wm_clear_keys();

// Keyboard IRQ1 handler (C)
void keyboard_handler(void) {
    static int extended = 0;
    uint8_t scancode = inb(PS2_DATA_PORT);
    // Ignore special responses
    if (scancode == 0xFA || scancode == 0xFE || scancode == 0x00 || scancode == 0xAA) {
        outb(0x20, 0x20);
        return;
    }
    if (scancode == 0xE0 || scancode == 0xE1) {
        extended = scancode;
        outb(0x20, 0x20);
        return;
    }
    // Ignore break codes (key release)
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }
    // Only handle make codes (key press)
    static const char scancode_map[200] = {
        0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,'\t',
        'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
        'd','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v',
        'b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    char c = 0;
    if (!extended) {
        if (scancode < 128) c = scancode_map[scancode];
    } else {
        extended = 0;
    }
    if (c && c < 128 && c > 32) {
        wm_update_key(c);
    }
    outb(0x20, 0x20); // Send EOI to PIC
}

// Mouse IRQ12 handler (C)
void mouse_handler(void) {
    static uint8_t packet[3];
    static int packet_index = 0;
    static int mouse_x = 400; // Start at center
    static int mouse_y = 300;
    extern struct framebuffer_info fb;
    int screen_w = (fb.width > 0) ? fb.width : 800;
    int screen_h = (fb.height > 0) ? fb.height : 600;
    uint8_t data = inb(PS2_DATA_PORT);
    // Enforce packet sync: first byte must have bit 3 set
    if (packet_index == 0 && !(data & 0x08)) {
        outb(0xA0, 0x20); // EOI to slave PIC
        outb(0x20, 0x20); // EOI to master PIC
        return;
    }
    packet[packet_index++] = data;
    if (packet_index == 3) {
        uint8_t flags = packet[0];
        int dx = (int8_t)packet[1];
        int dy = (int8_t)packet[2];
        // Ignore packet if overflow bits set
        if ((flags & 0x40) || (flags & 0x80)) {
            packet_index = 0;
            outb(0xA0, 0x20);
            outb(0x20, 0x20);
            return;
        }
        dy = -dy;
        int new_x = mouse_x + dx;
        int new_y = mouse_y + dy;
        // Clamp mouse_x and mouse_y to framebuffer bounds
        if (new_x < 0) new_x = 0;
        if (new_x >= screen_w) new_x = screen_w - 1;
        if (new_y < 0) new_y = 0;
        if (new_y >= screen_h) new_y = screen_h - 1;
        mouse_x = new_x;
        mouse_y = new_y;
        int left = (flags & 0x01) ? 1 : 0;
        wm_update_mouse(mouse_x, mouse_y, left);
        packet_index = 0;
    }
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

// -----------------------------------------------------------------------------
// Kernel entry point
// -----------------------------------------------------------------------------
struct framebuffer_info fb;

void kernel_main(void)
{
    serial_init();
    // term_init();
    ansi_clearhome();

    pic_remap();
    idt_init();
    pit_init(1000);
    idt_set_gate(32, (uint32_t)timer_handler_asm, 0x08, 0x8E);
    setup_syscalls();
    // Register keyboard and mouse handlers
    idt_set_gate(33, (uint32_t)keyboard_handler_asm, 0x08, 0x8E); // IRQ1
    idt_set_gate(44, (uint32_t)mouse_handler_asm, 0x08, 0x8E);    // IRQ12
    ps2_init();
    asm volatile("sti");

    extern unsigned char initfs_tar[];
    tarfs_init((const char *)initfs_tar);
    framebuffer_set_mode(800, 600, 32); // Set mode before detection
    framebuffer_detect(&fb);
    if (!fb.found) {
        serial_write_str("No QEMU framebuffer detected.");
        while (1) {
            asm volatile("hlt"); // Halt if no framebuffer found
        }
    }
    // --- Start window manager main loop ---
    window_manager_mainloop();

    // When window manager exits, switch to classic shell
    ansi_clearhome();
    term_init();
    shell_run();
    // Optionally halt after shell exits
    while (1) {
        asm volatile("hlt");
    }
}
