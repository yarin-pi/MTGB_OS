#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "std.h"
#include "list_head.h"
struct kprocess
{
    unsigned int pid;
    unsigned int num_threads;
    struct kthread *threads;
    struct list_head list;
};
struct kthread
{
    unsigned int tid;
    unsigned int parent_pid;
};
void scheduler_init(void);
void scheduler_next(void);
struct kprocess *init_task(unsigned int ppid);
void schedule_thread(struct kthread *thread);
void switch_thread(struct kthread *old, struct kthread *new);
#endif SCHEDULER_H