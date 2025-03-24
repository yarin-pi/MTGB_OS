#include "scheduler.h"
#include "vm.h"
struct kthread* current_task_TCB = 0;
struct kthread* first_ready = 0;
struct kthread* last_ready = 0;

struct kthread* first_sleep = 0;
struct kthread* last_sleep;

static uint32_t next_pid = 1;
static uint32_t next_tid = 1;

uint32_t IRQ_disable_counter = 0;
uint32_t postpone_task_switches_counter = 0;
uint32_t task_switches_postponed_flag = 0;

void switch_to_task_wrapper(struct kthread* task)
{
    if(postpone_task_switches_counter != 0)
    {
        task_switches_postponed_flag = 1;
        return;
    }
    if(task->tid == 333)
    {
        time_slice_remaining = 0;
    }
    else
    {
        time_slice_remaining = TIME_SLICE_LENGTH;

    }
    switch_to_task(task);

}
void kernel_idle_task(void) {
    for(;;) {
        HLT();
    }
}
void lock_scheduler(void) {
    asm volatile("cli");
    IRQ_disable_counter++;
}

void unlock_scheduler(void) {
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0) {
        asm volatile("sti");
    }
}

void lock_stuff(void) {
        asm volatile("cli");
        IRQ_disable_counter++;
        postpone_task_switches_counter++;
}
void unlock_stuff(void) {
    
    postpone_task_switches_counter--;
    if(postpone_task_switches_counter == 0) {
        if(task_switches_postponed_flag != 0) {
            task_switches_postponed_flag = 0;
            schedule();
        }
    }
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0) {
        asm volatile("sti");
    }
    
    }
    
void schedule(void)
{
    if(postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }
    if( first_ready != 0) {
        struct kthread * task = first_ready;
        first_ready = task->next;
        if(task->tid == 333)
        {
            struct kthread* idle = task;
            if(first_ready != 0)
            {
                task = first_ready;
                idle->next = task->next;
                first_ready = idle;
            }
            else if(current_task_TCB->s == RUNNING)
            {
                return;
            }
            else{

            }
        }
        switch_to_task(task);
    }
}
void unblock_task(struct kthread * task) {
    lock_scheduler();
    if((first_ready == 0) || (current_task_TCB->tid == 333)) {

        // Only one task was running before, so pre-empt

        switch_to_task(task);
    } else {
        // There's at least one task on the "ready to run" queue already, so don't pre-empt

        last_ready->next = task;
        last_ready = task;
    }
    unlock_scheduler();
}

