
#include "clock.h"
#include "idt.h"

volatile uint32_t time_since_boot = 0;
volatile uint32_t time_between_ticks = 10000000;

void pit_set_frequency(uint32_t frequency)
{
    pit_ticks = 0x0;
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / (frequency ? frequency : 1));
    if (divisor == 0)
        divisor = 1;                                 // Ensure valid divisor
    outb(PIT_COMMAND_PORT, 0x36);                    // Send command byte (8-bit)
    outb(PIT_CHANNEL_0_DATA, divisor & 0xFF);        // Send low byte
    outb(PIT_CHANNEL_0_DATA, (divisor >> 8) & 0xFF); // Send high byte
}

__attribute__((interrupt, target("general-regs-only"))) void pit_isr(struct interrupt_frame *frame)
{

    struct kthread* next_task = 0;
    struct kthread* this_task = 0;

    lock_stuff();
    time_since_boot += time_between_ticks;

    next_task = first_sleep;
    first_sleep = 0;

    while (next_task != 0)
    {
        this_task = next_task;
        next_task = this_task->next;
        if(this_task->sleep_expiry <= pit_ticks)
        {
            unblock_task(this_task);
        }
        else
        {
            this_task->next = first_sleep;
            first_sleep = this_task;
        }
    }

    if(time_slice_remaining != 0)
    {
        if(time_slice_remaining <= time_between_ticks)
        {
            schedule();
        }
        else
        {
            time_slice_remaining -= time_between_ticks;
        }
    }

    unlock_stuff();
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
    set_idt_entry(32, (uint32_t)HIGHER_HALF(pit_isr), 0x08, 0x8E);

    pit_set_frequency(100);
}
int return_tick()
{
    return pit_ticks;
}