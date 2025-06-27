#include <stdint.h>
#include "interrupt_handler.h"

#define IDT_ENTRIES 256

// IDT entry structure
struct idt_entry
{
    uint16_t base_low;
    uint16_t sel;    // Kernel segment selector
    uint8_t always0; // Always set to 0
    uint8_t flags;   // Flags (type and privilege level)
    uint16_t base_high;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Declare the IDT and its pointer
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

// Load the IDT (defined in assembly)
extern void idt_load(uint32_t);

// Set an entry in the IDT
void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Initialize the IDT
void idt_init()
{
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;

    // Clear the IDT
    for (int i = 0; i < IDT_ENTRIES; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    // Install catch-all handlers for all 256 vectors
    for (int i = 0; i < IDT_ENTRIES; i++) {
        // Use a function pointer array trick to get the address of catch_handler_N
        extern void (*const catch_handler_table[])(void *);
        idt_set_gate(i, (uint32_t)catch_handler_table[i], 0x08, 0x8E);
    }
    // Load the IDT
    idt_load((uint32_t)&idtp);
}