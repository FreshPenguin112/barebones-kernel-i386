#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags);
void idt_init();
extern void idt_load(uint64_t idt_ptr);
extern void default_int_handler_errcode_asm();

// Expose IDT structures for debugging
#define IDT_ENTRIES 256
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));
extern struct idt_entry idt[IDT_ENTRIES];
extern struct idt_ptr idtp;

#endif