#ifndef PROCESS_H
#define PROCESS_H

#include "std.h" // Include your standard definitions
#define MAX_THREADS 8
#define MAX_PROCESSES 256

// Process state enum (you can define READY, RUNNING, BLOCKED, etc.)
typedef enum
{
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} state;

// Process structure
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

// Function prototypes
void createProcess(uint32_t parent_pid, uint32_t priority, uint32_t memory_size);
void killProcess(uint32_t processID);
void suspendProcess(uint32_t processID);
void resumeProcess(uint32_t processID);
void allocateMemory(uint32_t processID, uint32_t size);
void addThread(uint32_t processID);
void removeThread(uint32_t processID);
uint32_t listThreads(uint32_t processID); // Returns the number of threads in a process

#endif // PROCESS_H
