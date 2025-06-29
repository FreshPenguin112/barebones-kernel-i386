#include <stdint.h>
#include "tss.h"
#include "serial.h"

// Extern GDT from assembly
extern uint64_t gdt64[];

// Correctly fill a 64-bit TSS descriptor in the GDT
void gdt_fill_tss_descriptor(void *tss_ptr, uint16_t tss_size) {
    uint64_t base = (uint64_t)tss_ptr;
    uint32_t limit = tss_size - 1;

    uint64_t desc_low = 0;
    uint64_t desc_high = 0;

    desc_low  = (limit & 0xFFFFULL);                // Limit 0:15
    desc_low |= ((base & 0xFFFFFFULL) << 16);       // Base 0:23
    desc_low |= (0x9ULL << 40);                     // Type: 64-bit available TSS (0x9)
    desc_low |= (1ULL << 47);                       // Present
    desc_low |= (((uint64_t)((limit >> 16) & 0xF)) << 48);        // Limit 16:19 (fix: cast to 64-bit before shift)
    desc_low |= (((uint64_t)((base >> 24) & 0xFF)) << 56);        // Base 24:31 (fix: cast to 64-bit before shift)

    desc_high = (base >> 32) & 0xFFFFFFFFULL;       // Base 32:63
    // Upper 32 bits of desc_high must be zero

    serial_write_string("[DBG] gdt_fill_tss_descriptor base: ");
    serial_write_hex(base);
    serial_write_string("\r\n[DBG] gdt_fill_tss_descriptor desc_low: ");
    serial_write_hex(desc_low);
    serial_write_string("\r\n[DBG] gdt_fill_tss_descriptor desc_high: ");
    serial_write_hex(desc_high);
    serial_write_string("\r\n");

    gdt64[3] = desc_low;
    gdt64[4] = desc_high;
}
