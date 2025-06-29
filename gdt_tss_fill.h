#ifndef GDT_TSS_FILL_H
#define GDT_TSS_FILL_H
#include <stdint.h>
#include "tss.h"
void gdt_fill_tss_descriptor(void *tss_ptr, uint16_t tss_size);
#endif
