#include "scheduler.h"
#include "vm.h"
struct kprocess *run_queue = 0;
struct kprocess *wait_queue = 0;
struct kthread* current_task_TCB = 0;
struct kprocess *cur_process = 0;

static uint32_t next_pid = 1;
static uint32_t next_tid = 1;

static int scheduler_enabled = 0;

void scheduler_init(void)
{
    cur_process = init_task();
    current_task_TCB = create_thread(cur_process, 0);
    current_task_TCB->stack = 0;
    switch_thread(current_task_TCB, current_task_TCB);
    current_task_TCB->s = RUNNING;
    scheduler_enabled = 1;
}

void scheduler_next(void)
{
    if (scheduler_enabled == 0)
        return;

    if (run_queue == 0)
        return;

    struct kthread *thread = current_task_TCB;
    struct kprocess *proc = cur_process;

    update_time_slice();

    if (current_task_TCB->timeSlice == 0)
    {
        schedule_thread(current_task_TCB);
        current_task_TCB = run_queue;
        run_queue = run_queue->next;
        current_task_TCB->next = 0;
        current_task_TCB->s = RUNNING;
        switch_thread(thread, current_task_TCB);
    }
}

void schedule_thread(struct kthread *thread)
{
    if (run_queue == 0)
    {
        run_queue = thread;
        return;
    }

    struct kthread *temp = run_queue;
    while (temp->next != 0)
        temp = temp->next;
    temp->next = thread;
    thread->s = READY;
}

void update_time_slice(void)
{
    if (current_task_TCB != 0)
    {
        if (current_task_TCB->timeSlice > 0)
        {
            current_task_TCB->timeSlice--;
        }
        else
        {
            reset_time_slice(current_task_TCB);
        }
    }

    if (cur_process != 0)
    {
        if (cur_process->timeSlice > 0)
        {
            cur_process->timeSlice--;
        }
        else
        {
            cur_process->timeSlice = 10;
        }
    }
}

void reset_time_slice(struct kthread *thread)
{
    thread->timeSlice = 5;
    schedule_thread(thread);
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

    schedule_thread(thread);
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