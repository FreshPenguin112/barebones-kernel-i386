#include "pit.h"
#include "io.h"

#define PIT_FREQ 1193182
#define PIT_CHAN0 0x40
#define PIT_CMD  0x43

void pit_init(uint32_t freq) {
    uint16_t divisor = (uint16_t)(PIT_FREQ / freq);
    outb(PIT_CMD, 0x36); // binary, mode 3, lobyte/hibyte, channel 0
    outb(PIT_CHAN0, divisor & 0xFF);
    outb(PIT_CHAN0, (divisor >> 8) & 0xFF);
}