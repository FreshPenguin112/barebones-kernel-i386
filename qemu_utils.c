#include "qemu_utils.h"
#include <stdint.h>

extern void kernel_print(const char *str);

// QEMU exit via ISA debug port (0x501)
void qemu_halt_exit(int code)
{
    // QEMU standard: write 0x2000 | (code & 0xFF) to port 0x501
    uint16_t port = 0x501;
    uint32_t value = 0x2000 | (code & 0xFF);
    kernel_print("\n");
    asm volatile("outl %0, %1" ::"a"(value), "Nd"(port));
    // Infinite loop in case QEMU doesn't exit
    for (;;)
    {
        asm volatile("hlt");
    }
}

// Add more QEMU utility functions here as needed