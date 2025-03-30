#include "clock.h"
#include "idt.h"
#include "std.h"
#include "scheduler.h"
volatile uint32_t time_between_ticks = 10;

void pit_set_frequency(uint32_t frequency)
{

    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / (frequency ? frequency : 1));
    if (divisor == 0)
        divisor = 1;                                 // Ensure valid divisor
    outb(PIT_COMMAND_PORT, 0x36);                    // Send command byte (8-bit)
    outb(PIT_CHANNEL_0_DATA, divisor & 0xFF);        // Send low byte
    outb(PIT_CHANNEL_0_DATA, (divisor >> 8) & 0xFF); // Send high byte
}

__attribute__((interrupt, target("general-regs-only"))) void pit_isr(struct interrupt_frame *frame)
{

    struct kthread *next_task = 0;
    struct kthread *this_task = 0;
    if (scheduler_enabled)
    {

        if (frame->cs & 0x3 && frame->sp < 0xf0000000)
        {
            // kprintf("i want to kill myself isr \n");
            current_task_TCB->eip = frame->ip;
            current_task_TCB->stack = frame->sp;
            // kprintf("esp isr: %p \n",frame->sp );
        }

        lock_stuff();
    }
    time_since_boot += time_between_ticks;
    if (scheduler_enabled)
    {
        next_task = first_sleep;
        first_sleep = 0;

        while (next_task != 0)
        {
            this_task = next_task;
            next_task = this_task->next;
            if (this_task->sleep_expiry <= time_since_boot)
            {
                if (this_task->tid == 333 && this_task->next)
                {
                    unblock_task(this_task->next);
                }
                else
                {
                    unblock_task(this_task);
                }
            }
            else
            {
                this_task->next = first_sleep;
                first_sleep = this_task;
            }
        }

        if (time_slice_remaining != 0)
        {
            if (time_slice_remaining <= time_between_ticks)
            {
                struct kthread *curr = first_ready;
                while (curr->next != 0)
                {
                    curr = curr->next;
                }
                curr->next = current_task_TCB;
                current_task_TCB->next = 0;
                schedule();
            }
            else
            {
                time_slice_remaining -= time_between_ticks;
            }
        }
    }

    outb(0x20, 0x20);
    if (scheduler_enabled)
    {
        unlock_stuff();
    }
}

void clock_init()
{
    set_idt_entry(32, (uint32_t)HIGHER_HALF(pit_isr), 0x08, 0x8E);

    pit_set_frequency(100);
}
