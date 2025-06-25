#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_init();
extern void idt_load(uint32_t idt_ptr); // Declare the assembly function
void default_isr_handler(int vector);

#endif