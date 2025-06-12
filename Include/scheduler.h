#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "std.h"
#define MAX_THREADS 32
#define TIME_SLICE_LENGTH 10
#define JUMP_TO(address) \
    asm volatile("jmp *%0" ::"r"(address))
struct kthread
{
    void *cr3;
    state s;
    uint32_t *stack;
    uint32_t stack_top;
    uint32_t *eip;
    uint32_t tid;
    uint32_t isUser;
    uint32_t sleep_expiry;
    uint32_t time_slice;
    struct kthread *next;
};

struct kprocess
{
    void *arg;
    state s;
    uint32_t pid;
    uint32_t num_threads;
    uint32_t timeSlice;
    struct kprocess *next;
    struct kthread *threads[MAX_THREADS];
    struct kprocess *parent;
    struct kprocess *children; // Pointer to child processes
};

typedef struct
{
    int max_count;
    int current_count;
    struct kthread *first_waiting_task;
    struct kthread *last_waiting_task;
} SEMAPHORE;
extern volatile uint32_t time_since_boot;
extern struct kthread *first_terminated;
extern struct kthread *first_ready;
void init_scheduler(void);
struct kthread *init_task(uint32_t *phy_cr3, uint32_t *stack, uint32_t *entry_point, int isIdle, int isUser);
extern void switch_to_task(struct kthread *tcb);
extern struct kthread *current_task_TCB;
extern struct kthread *first_sleep;
extern struct kthread *last_sleep;
extern uint32_t postpone_task_switches_counter;
extern uint32_t task_switches_postponed_flag;
void lock_stuff(void);
void kernel_idle_task(void);
void cleaner_t(void);
void unlock_stuff(void);
void lock_scheduler(void);
void unlock_scheduler(void);
void block_task(int reason);
void terminate_task(void);
void unblock_task(struct kthread *task);
void schedule(void);

#endif SCHEDULER_H