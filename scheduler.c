#include "scheduler.h"
#include "vm.h"
#include "gdt.h"
#include "print.h"
volatile uint32_t time_since_boot = 0;
struct kthread *current_task_TCB = 0;
struct kthread *first_ready = 0;
struct kthread *last_ready = 0;
struct kthread *first_sleep = 0;
struct kthread *first_terminated = 0;
struct kthread *cleaner_task = 0;
static uint32_t next_tid = 1;

uint32_t IRQ_disable_counter = 0;
uint32_t postpone_task_switches_counter = 0;
uint32_t task_switches_postponed_flag = 0;
void init_scheduler(void)
{
    cleaner_task = init_task(page_directory, get_esp(), cleaner_t, 0, 0);
    last_ready = init_task(page_directory, get_esp(), kernel_idle_task, 1, 0);
}

struct kthread *init_task(uint32_t *vir_cr3, uint32_t *stack, uint32_t *entry_point, int isIdle, int isUser)
{
    struct kthread *thread = (struct kthread *)kalloc(sizeof(struct kthread));
    thread->cr3 = vir_cr3;
    thread->eip = entry_point;
    thread->stack_top = stack;
    thread->stack = stack;
    thread->s = READY;
    thread->isUser = isUser;
    if (isIdle)
    {
        thread->tid = 333;
    }
    else
    {
        thread->tid = next_tid++;
    }
    struct kthread *curr = first_ready;
    if (!first_ready)
    {
        first_ready = thread;
    }
    else
    {
        while (curr->next)
        {
            curr = curr->next;
        }
        curr->next = thread;
    }

    return thread;
}
void switch_to_task_wrapper(struct kthread *task)
{
    asm volatile("cli");
    if (postpone_task_switches_counter != 0)
    {
        task_switches_postponed_flag = 1;
        return;
    }
    if (task->tid == 333)
    {
        time_slice_remaining = 0;
    }
    else
    {
        time_slice_remaining = TIME_SLICE_LENGTH;
    }
    switch_to_task(task);
    if (task->isUser)
    {
        jump_usermode(task->eip, task->cr3, task->stack);
    }
    else
    {
        JUMP_TO(HIGHER_HALF(task->eip));
    }
}
void kernel_idle_task(void)
{
    asm volatile("sti");
    for (;;)
    {
        asm volatile("hlt");
    }
}
void lock_scheduler(void)
{
    asm volatile("cli");
    IRQ_disable_counter++;
}

void unlock_scheduler(void)
{
    IRQ_disable_counter--;
    if (IRQ_disable_counter == 0)
    {
        asm volatile("sti");
    }
}

void lock_stuff(void)
{
    asm volatile("cli");
    IRQ_disable_counter++;
    postpone_task_switches_counter++;
}
void unlock_stuff(void)
{

    postpone_task_switches_counter--;
    if (postpone_task_switches_counter == 0)
    {
        if (task_switches_postponed_flag != 0)
        {
            task_switches_postponed_flag = 0;
            schedule();
        }
    }
    IRQ_disable_counter--;
    if (IRQ_disable_counter == 0)
    {
        asm volatile("sti");
    }
}

void schedule(void)
{
    if (postpone_task_switches_counter != 0)
    {
        task_switches_postponed_flag = 1;
        return;
    }
    if (first_ready != 0)
    {
        struct kthread *task = first_ready;
        first_ready = task->next;
        if (task->tid == 333)
        {
            struct kthread *idle = task;
            if (first_ready != 0)
            {
                task = first_ready;
                idle->next = task->next;
                first_ready = idle;
            }
            else if (current_task_TCB->s == RUNNING)
            {
                return;
            }
            else
            {
            }
        }

        switch_to_task_wrapper(task);
    }
}
void block_task(int reason)
{
    lock_scheduler();
    current_task_TCB->s = reason;

    if (reason == BLOCKED)
    {
        if (!first_sleep)
        {
            first_sleep = current_task_TCB;
        }
        struct kthread *curr = first_sleep;
        while (curr->next)
        {
            curr = curr->next;
        }
        curr->next = current_task_TCB;
        current_task_TCB->next = 0;
    }

    schedule();
    unlock_scheduler();
}

void unblock_task(struct kthread *task)
{
    lock_scheduler();
    if ((first_ready == 0) || (current_task_TCB->tid == 333))
    {

        // Only one task was running before, so pre-empt

        switch_to_task_wrapper(task);
    }
    else
    {
        // There's at least one task on the "ready to run" queue already, so don't pre-empt

        struct kthread *curr = first_ready;
        while (curr->next)
        {
            curr = curr->next;
        }
        curr->next = task;
        task->next = 0;
    }
    unlock_scheduler();
}

void terminate_task(void)
{

    lock_stuff();

    lock_scheduler();
    current_task_TCB->next = first_terminated;
    first_terminated = current_task_TCB;
    struct kthread *curr = first_ready;
    while (curr->next != 0)
    {
        if (curr->next == current_task_TCB)
        {
            curr->next = current_task_TCB->next;
        }
        curr = curr->next;
    }
    unlock_scheduler();

    block_task(TERMINATED);

    unblock_task(cleaner_task);
}
void cleaner_t(void)
{
    struct kthread *task;
    lock_stuff();
    while (first_terminated != 0)
    {
        task = first_terminated;
        first_terminated = task->next;
        clean_up_terminated_task(task);
    }
    block_task(BLOCKED);
    unlock_stuff();
}
void clean_up_terminated_task(struct kthread *task)
{
    if (task->isUser)
    {
        kfree(task->cr3, 4);
    }
    kfree(task, sizeof(struct kthread));
}
SEMAPHORE *create_semaphore(int max_count)
{
    SEMAPHORE *semaphore;
    semaphore = kalloc(sizeof(SEMAPHORE));
    if (semaphore != 0)
    {
        semaphore->max_count = max_count;
        semaphore->current_count = 0;
        semaphore->first_waiting_task = 0;
        semaphore->last_waiting_task = 0;
    }
}
SEMAPHORE *create_mutex(void)
{
    return create_semaphore(1);
}

void acquire_semaphore(SEMAPHORE *semaphore)
{
    lock_stuff();
    if (semaphore->current_count < semaphore->max_count)
    {
        semaphore->current_count++;
    }
    else
    {
        current_task_TCB->next = 0;
        if (semaphore->first_waiting_task == 0)
        {
            semaphore->first_waiting_task = current_task_TCB;
        }
        else
        {
            semaphore->last_waiting_task->next = current_task_TCB;
        }
        semaphore->last_waiting_task = current_task_TCB;
        block_task(WAITING_FOR_LOCK);
    }
    unlock_stuff();
}
void acquire_mutex(SEMAPHORE *semaphore)
{
    acquire_semaphore(semaphore);
}

void release_semaphore(SEMAPHORE *semaphore)
{
    lock_stuff();
    if (semaphore->first_waiting_task != 0)
    {
        struct kthread *task = semaphore->first_waiting_task;
        semaphore->first_waiting_task = task->next;
        unblock_task(task);
    }
    else
    {
        semaphore->current_count--;
    }
    unlock_stuff();
}

void release_mutex(SEMAPHORE *semaphore)
{
    release_semaphore(semaphore);
}