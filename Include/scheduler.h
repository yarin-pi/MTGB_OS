#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "std.h"
#define MAX_THREADS 32
struct kthread
{
    state s;
    uint32_t *stack;
    uint32_t tid;
    uint32_t parent_pid;
    uint32_t timeSlice;
    struct kthread *next;
    void *arg;
};

struct kprocess
{
    state s;
    uint32_t pid;
    uint32_t num_threads;
    uint32_t timeSlice;
    struct kthread *threads[MAX_THREADS];
    void *arg;
};

// Scheduler functions
void scheduler_init(void);
void scheduler_next(void);
void schedule_thread(struct kthread *thread);
void switch_thread(struct kthread *old, struct kthread *new);
struct kprocess *init_task(void);
void create_process(void);
void destroy_process(struct kprocess *proc);

// Thread management functions
struct kthread *create_thread(struct kprocess *proc, void *entry);
void destroy_thread(struct kthread *thread);
#endif SCHEDULER_H