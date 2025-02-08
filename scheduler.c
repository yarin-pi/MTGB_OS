#include "scheduler.h"
static struct kprocess *run_queue = NULL;
static struct kprocess *wait_queue = NULL;
static struct kthread *cur_thread = NULL;
static struct kprocess *cur_process = NULL;

static uint32_t next_pid = 1; // Process ID counter
static uint32_t next_tid = 1; // Thread ID counter

static int scheduler_enabled = 0;

void scheduler_init(void)
{
    cur_process = init_task();                     // Initialize the initial process (idle process)
    cur_thread = create_thread(cur_process, NULL); // Create the first thread (init thread)
    cur_thread->stack = NULL;
    switch_thread(cur_thread, cur_thread);
    cur_thread->s = THREAD_RUNNING;
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

    // Schedule next thread in the process's run queue
    schedule_thread(cur_thread);
    cur_thread = run_queue;
    run_queue = run_queue->next;

    cur_thread->next = NULL;
    cur_thread->s = THREAD_RUNNING;

    // Switch to the next thread
    switch_thread(thread, cur_thread);
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
    thread->s = THREAD_READY;
}

struct kprocess *init_task(void)
{
    struct kprocess *proc = malloc(sizeof(struct kprocess));
    if (!proc)
        return NULL;

    proc->pid = next_pid++;
    proc->num_threads = 0;
    proc->timeSlice = 10;
    proc->s = PROCESS_READY;
    proc->next = NULL;
    return proc;
}

void create_process(void)
{
    struct kprocess *proc = malloc(sizeof(struct kprocess));
    if (!proc)
        return;

    proc->pid = next_pid++;
    proc->num_threads = 0;
    proc->timeSlice = 10;
    proc->s = PROCESS_READY;
    proc->next = run_queue;
    run_queue = proc;
}

void destroy_process(struct kprocess *proc)
{
    if (!proc)
        return;

    for (uint32_t i = 0; i < proc->num_threads; i++)
        destroy_thread(proc->threads[i]);

    free(proc);
}

struct kthread *create_thread(struct kprocess *proc, void *entry)
{
    struct kthread *thread = malloc(sizeof(struct kthread));
    if (!thread)
        return NULL;

    thread->tid = next_tid++;
    thread->parent_pid = proc ? proc->pid : 0;
    thread->timeSlice = 5;
    thread->stack = malloc(4096); // Allocate stack space
    thread->s = THREAD_READY;
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

    free(thread->stack);
    free(thread);
}

void switch_thread(struct kthread *old, struct kthread *new)
{
    if (old == new)
        return;

    // Context switching logic (simplified)
    __asm__ volatile(
        "movl %0, %%esp\n"
        :
        : "r"(new->stack));

    cur_thread = new;
}
