#ifndef PIT_H
#define PIT_H

#include <stdint.h>
void pit_init(uint32_t freq);
extern volatile uint64_t timer_ticks;

#endif