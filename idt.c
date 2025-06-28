#include <stdint.h>
#include "io.h"
#include "serial.h"
#define IDT_ENTRIES 256

// x86_64 IDT entry structure
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Declare the IDT and its pointer
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

// Load the IDT (defined in assembly)
extern void idt_load(uint64_t);

// Default interrupt handler
void default_int_handler(void);
extern void default_int_handler_asm();
extern void default_int_handler_errcode_asm();

// Minimal interrupt handler for testing
void minimal_int_handler(void) {
    serial_write_char('M'); // Output 'M' for minimal handler
    outb(0x20, 0x20); // Send EOI to PIC (for IRQs)
}
extern void minimal_int_handler_asm(void);

// Set an entry in the IDT
void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].offset_mid = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

// Initialize the IDT
void idt_init() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint64_t)&idt;
    // Get current CS selector
    uint16_t cs_selector = 0;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs_selector));
    // Remove debug output for production
    // Exceptions with error code: 8, 10, 11, 12, 13, 14, 17
    int errcode_vecs[] = {8, 10, 11, 12, 13, 14, 17};
    for (int i = 0; i < IDT_ENTRIES; i++) {
        int is_errcode = 0;
        for (unsigned j = 0; j < sizeof(errcode_vecs)/sizeof(errcode_vecs[0]); j++) {
            if (i == errcode_vecs[j]) { is_errcode = 1; break; }
        }
        if (is_errcode)
            idt_set_gate(i, (uint64_t)default_int_handler_errcode_asm, cs_selector, 0x8E);
        else
            idt_set_gate(i, (uint64_t)default_int_handler_asm, cs_selector, 0x8E);
    }
    // Re-set custom handlers after defaulting all
    extern void timer_handler_asm();
    extern void syscall_handler_asm();
    idt_set_gate(32, (uint64_t)timer_handler_asm, cs_selector, 0x8E); // IRQ0
    idt_set_gate(0x80, (uint64_t)syscall_handler_asm, cs_selector, 0xEE); // Syscall, DPL=3
    idt_load((uint64_t)&idtp);
}