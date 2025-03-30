#ifndef CLOCK_H
#define CLOCK_H

#include "std.h"
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL_0_DATA 0x40
#define PIT_FREQUENCY 1193182

void pit_set_frequency(uint32_t frequency);
void pit_isr(struct interrupt_frame *frame);
void wait_ticks(uint32_t ticks);
void clock_init();
int return_tick();
#endif CLOCK_H // !CLOCK_H
