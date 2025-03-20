#include "scheduler.h"
#include "vm.h"
struct kthread* current_task_TCB = 0;
struct kthread* first_ready = 0;
struct kthread* last_ready = 0;

static uint32_t next_pid = 1;
static uint32_t next_tid = 1;

uint32_t IRQ_disable_counter = 0;
uint32_t postpone_task_switches_counter = 0;
uint32_t task_switches_postponed_flag = 0;
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
        switch_to_task(task);
    }
}
void unblock_task(struct kthread * task) {
    lock_scheduler();
    if(first_ready == 0) {

        // Only one task was running before, so pre-empt

        switch_to_task(task);
    } else {
        // There's at least one task on the "ready to run" queue already, so don't pre-empt

        last_ready->next = task;
        last_ready = task;
    }
    unlock_scheduler();
}
void reset_time_slice(struct kthread *thread)
{
    thread->timeSlice = 5;
    //schedule_thread(thread);
}

struct kprocess *init_task(void)
{
    struct kprocess *proc = kalloc(sizeof(struct kprocess));
    if (!proc)
        return 0;

    proc->pid = next_pid++;
    proc->num_threads = 0;
    proc->timeSlice = 10;
    proc->s = READY;
    proc->next = 0;
    return proc;
}

void destroy_process(struct kprocess *proc)
{
    if (!proc)
        return;

    // Destroy child processes first
    struct kprocess *child = proc->children;
    while (child)
    {
        struct kprocess *next = child->next;
        destroy_process(child);
        child = next;
    }

    for (uint32_t i = 0; i < proc->num_threads; i++)
    {
        destroy_thread(proc->threads[i]);
    }

    kfree(proc, sizeof(struct kprocess));
}

struct kthread *create_thread(struct kprocess *proc, void *entry)
{
    struct kthread *thread = kalloc(sizeof(struct kthread));
    if (!thread)
        return 0;

    thread->tid = next_tid++;
    thread->parent_pid = proc ? proc->pid : 0;
    thread->timeSlice = 5;
    thread->stack = kalloc(4096);
    thread->s = READY;
    thread->next = 0;
    thread->arg = entry;

    if (proc)
    {
        proc->threads[proc->num_threads++] = thread;
    }

    //schedule_thread(thread);
    return thread;
}

void destroy_thread(struct kthread *thread)
{
    if (!thread)
        return;

    kfree(thread->stack, 4096);
    kfree(thread, sizeof(struct kthread));
}

void switch_thread(struct kthread *old, struct kthread *new)
{
    if (old == new)
        return;

    __asm__ volatile(
        "movl %0, %%esp\n"
        :
        : "r"(new->stack));

    current_task_TCB = new;
}

struct kprocess *fork_process(struct kprocess *parent)
{
    struct kprocess *child = kalloc(sizeof(struct kprocess));
    if (!child)
        return 0;

    child->pid = next_pid++;
    child->num_threads = 0;
    child->timeSlice = parent->timeSlice;
    child->s = READY;
    child->next = 0;
    child->parent = parent; // Assign parent process

    // Add to parent's child list
    child->children = parent->children;
    parent->children = child;

    return child;
}