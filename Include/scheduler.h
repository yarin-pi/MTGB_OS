#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "std.h"
#include "list_head.h"
struct kprocess
{
    unsigned int pid;
    unsigned int parent_pid;
    int nice;
    unsigned int num_threads;
    struct kthread *threads;
    struct list_head list;
};
void scheduler_init(void);
void scheduler_next(void);
struct pcb *init_task(int nice, unsigned int ppid);
void schedule_thread(struct tcb *thread);
void switch_thread(struct tcb *old, struct tcb *new);
#endif SCHEDULER_H