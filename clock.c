#include "clock.h"
#include "std.h"
volatile uint32_t pit_ticks = 0;

void pit_set_frequency(uint32_t frequency)
{
    pit_ticks = 0x0;
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    // Send the command byte
    outl(PIT_COMMAND_PORT, 0x36); // Channel 0, Low/High byte, Mode 3 (square wave generator)

    // Send the frequency divisor
    outl(PIT_CHANNEL_0_DATA, divisor & 0xFF); // Low byte
    outl(PIT_CHANNEL_0_DATA, (divisor >> 8) & 0xFF);
}

__attribute__((interrupt, target("general-regs-only"))) void pit_isr(struct interrupt_frame *frame)
{
    pit_ticks++;
    outl(0x20, 0x20);
}

void wait_ticks(uint32_t ticks)
{
    uint32_t s = pit_ticks;
    while ((pit_ticks - s) < ticks)
    {
        asm volatile("hlt");
    }
}