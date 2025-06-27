#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags);
void idt_init();
extern void idt_load(uint64_t idt_ptr);
extern void default_int_handler_errcode_asm();

#endif