#include "tss.h"
#include "serial.h"
#include <string.h>
#include <stdint.h>

// Place TSS in the default data section (higher half, mapped by Limine)
struct tss64 tss __attribute__((aligned(16)));

void tss_install(void *df_stack, uint64_t df_stack_size) {
    memset(&tss, 0, sizeof(tss));
    // Set IST1 to point to the top of the double fault stack
    tss.ist1 = (uint64_t)df_stack + df_stack_size;
    tss.iomap_base = sizeof(struct tss64);
    // Serial debug: print selector value before ltr
    serial_write_string("[DBG] tss_install: loading TSS selector 0x18\r\n");
    // Load TSS selector (GDT entry 0x18)
    __asm__ volatile ("ltr %%ax" : : "a" (0x18));
    serial_write_string("[DBG] tss_install: ltr complete\r\n");
}

// Export the TSS for GDT setup
struct tss64 *get_tss_ptr() { return &tss; }
