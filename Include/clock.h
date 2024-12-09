#ifndef CLOCK_H
#define CLOCK_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL_0_DATA 0x40
#define PIT_FREQUENCY 1193182

void pit_set_frequency(uint32_t frequency);
void pit_isr();
void wait_ticks(uint32_t ticks);
#endif CLOCK_H// !CLOCK_H

