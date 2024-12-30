#ifndef PROCESS_H
#define PROCESS_H
#include "std.h"
#define MAX_THREADS 8

typedef struct Process
{
    uint32_t pid;              // Process ID
    uint32_t parent_pid;       // Parent process ID
    uint32_t priority;         // Priority of the process
    uint32_t *stack_pointer;   // Stack pointer
    uint32_t *program_counter; // Program counter
    state s;                   // Process state (e.g., READY, RUNNING, BLOCKED)
    uint32_t memory_base;      // Base address of process memory
    uint32_t memory_limit;     // Size/limit of process memory
    uint32_t num_of_threads;   // Number of threads
} Process;
#endif PROCESS_H
