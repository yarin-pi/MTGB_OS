#include "scheduler.h"
#include "vm.h"
static struct kprocess *run_queue = NULL;
static struct kprocess *wait_queue = NULL;
static struct kthread *cur_thread = NULL;
static struct kprocess *cur_process = NULL;

static uint32_t next_pid = 1;
static uint32_t next_tid = 1;

static int scheduler_enabled = 0;

void scheduler_init(void)
{
    cur_process = init_task();
    cur_thread = create_thread(cur_process, NULL);
    cur_thread->stack = NULL;
    switch_thread(cur_thread, cur_thread);
    cur_thread->s = RUNNING;
    scheduler_enabled = 1;
}

void scheduler_next(void)
{
    if (scheduler_enabled == 0)
        return;

    if (run_queue == NULL)
        return;

    struct kthread *thread = cur_thread;
    struct kprocess *proc = cur_process;

    update_time_slice();

    if (cur_thread->timeSlice == 0)
    {
        schedule_thread(cur_thread);
        cur_thread = run_queue;
        run_queue = run_queue->next;
        cur_thread->next = NULL;
        cur_thread->s = RUNNING;
        switch_thread(thread, cur_thread);
    }
}

void schedule_thread(struct kthread *thread)
{
    if (run_queue == NULL)
    {
        run_queue = thread;
        return;
    }

    struct kthread *temp = run_queue;
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = thread;
    thread->s = READY;
}

void update_time_slice(void)
{
    if (cur_thread != NULL)
    {
        if (cur_thread->timeSlice > 0)
        {
            cur_thread->timeSlice--;
        }
        else
        {
            reset_time_slice(cur_thread);
        }
    }

    if (cur_process != NULL)
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
        return NULL;

    proc->pid = next_pid++;
    proc->num_threads = 0;
    proc->timeSlice = 10;
    proc->s = READY;
    proc->next = NULL;
    return proc;
}

void destroy_process(struct kprocess *proc)
{
    if (!proc)
        return;

    for (uint32_t i = 0; i < proc->num_threads; i++)
        destroy_thread(proc->threads[i]);

    kfree(proc);
}

struct kthread *create_thread(struct kprocess *proc, void *entry)
{
    struct kthread *thread = kalloc(sizeof(struct kthread));
    if (!thread)
        return NULL;

    thread->tid = next_tid++;
    thread->parent_pid = proc ? proc->pid : 0;
    thread->timeSlice = 5;
    thread->stack = kalloc(4096);
    thread->s = READY;
    thread->next = NULL;
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

    cur_thread = new;
}
