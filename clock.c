#include "clock.h"
#include "std.h"
#include "idt.h"
volatile uint32_t pit_ticks = 0;

void pit_set_frequency(uint32_t frequency)
{
    pit_ticks = 0x0;
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);
    outb(PIT_COMMAND_PORT, 0x36);                    // Send command byte (8-bit)
    outb(PIT_CHANNEL_0_DATA, divisor & 0xFF);        // Send low byte
    outb(PIT_CHANNEL_0_DATA, (divisor >> 8) & 0xFF); // Send high byte
}
__attribute__((interrupt, target("general-regs-only"))) void pit_isr(struct interrupt_frame *frame)
{
    pit_ticks++;
    outb(0x20, 0x20);
}
void wait_ticks(uint32_t ticks)
{
    uint32_t s = pit_ticks;
    while ((pit_ticks - s) < ticks)
    {
        asm volatile("sti; hlt");
    }
}

void clock_init()
{
    pit_set_frequency(1000); // Set PIT to 1000 Hz (1ms per tick)
    set_idt_entry(32, (uint32_t)HIGHER_HALF(pit_isr), 0x08, 0x8E);
}