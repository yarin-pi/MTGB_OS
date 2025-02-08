#include "scheduler.h"
static struct kthread *run_queue = NULL;
static struct kthread *wait_queue = NULL;
static struct kthread *cur = NULL;

static int scheduler_enabled = 0;

void sched_init(void)
{
    cur = kthread_create(NULL, NULL);
    cur->stack = NULL;
    switch_thread(cur, cur);
    cur->state = THREAD_RUNNING;
    scheduler_enabled = 1;
}

void schedule_next(void)
{
    if (scheduler_enabled == 0)
        return;
    if (run_queue == NULL)
        return;

    struct kthread *thread = cur;
    schedule_thread(cur);
    cur = run_queue;
    run_queue = run_queue->next;
    cur->next = NULL;
    cur->state = THREAD_RUNNING;
    switch_thread(thread, cur);
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
    thread->state = THREAD_READY;
}
