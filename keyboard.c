#include "kernel.h"
#include "keyboard.h"
#include "io.h"

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KEYBUF_SIZE 64

static char keybuf[KEYBUF_SIZE];
static int keybuf_head = 0, keybuf_tail = 0;
static int shift = 0, ctrl = 0, alt = 0, caps = 0;

// US QWERTY scancode set 1
static const char scancode_table[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    // rest are zeros
};
static const char scancode_table_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    // rest are zeros
};

void keyboard_clear_buffer(void) {
    for (int i = 0; i < KEYBUF_SIZE; i++) keybuf[i] = 0;
    keybuf_head = 0;
    keybuf_tail = 0;
}

int keyboard_has_char(void) {
    return keybuf_head != keybuf_tail;
}

char keyboard_read_char(void) {
    if (keybuf_head == keybuf_tail) return 0;
    char c = keybuf[keybuf_tail];
    keybuf_tail = (keybuf_tail + 1) % KEYBUF_SIZE;
    return c;
}

static void keybuf_put(char c) {
    int next = (keybuf_head + 1) % KEYBUF_SIZE;
    if (next != keybuf_tail) {
        keybuf[keybuf_head] = c;
        keybuf_head = next;
    }
}

void keyboard_init(void) {
    // No initialization needed for basic PS/2 handler
}

void keyboard_handler(void) {
    if (!(inb(KBD_STATUS_PORT) & 1)) return;
    uint8_t sc = inb(KBD_DATA_PORT);

    // Ignore key releases
    if (sc & 0x80) {
        // Modifier releases
        if (sc == 0xAA || sc == 0xB6) shift = 0;
        if (sc == 0x9D) ctrl = 0;
        if (sc == 0xB8) alt = 0;
        return;
    }
    // Modifier presses
    if (sc == 0x2A || sc == 0x36) { shift = 1; return; }
    if (sc == 0x1D) { ctrl = 1; return; }
    if (sc == 0x38) { alt = 1; return; }
    if (sc == 0x3A) { caps = !caps; return; }

    char c = 0;
    if (sc < 128) {
        c = shift ? scancode_table_shift[sc] : scancode_table[sc];
        // Caps lock for letters
        if (c >= 'a' && c <= 'z') {
            if (caps ^ shift) c = c - 'a' + 'A';
        } else if (c >= 'A' && c <= 'Z') {
            if (caps ^ shift) c = c - 'A' + 'a';
        }
    }
    // Only buffer printable ASCII, Enter, Tab, and Backspace
    if ((c >= 32 && c <= 126) || c == '\n' || c == '\t' || c == '\b') {
        keybuf_put(c);
    }
}
