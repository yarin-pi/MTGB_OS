#include "process.h"
#define MAX_PROCESSES 256
Process processTable[MAX_PROCESSES] = {0};

// Find an available process slot
static int findAvailableProcessSlot()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processTable[i].pid == 0)
        { // Unused PID (0 means slot is free)
            return i;
        }
    }
    return -1; // No available slot
}

// Create a new process
void createProcess(uint32_t parent_pid, uint32_t priority, uint32_t memory_size)
{
    int slot = findAvailableProcessSlot();
    if (slot == -1)
    {
        return; // Process table is full, handle error elsewhere
    }

    Process *newProcess = &processTable[slot];
    newProcess->pid = slot + 1; // Assign PID (slot index + 1)
    newProcess->parent_pid = parent_pid;
    newProcess->priority = priority;
    newProcess->stack_pointer = 0;   // Will be set when the process starts
    newProcess->program_counter = 0; // Will be set when the process starts
    newProcess->s = READY;           // Default state
    newProcess->memory_base = (uint32_t)kalloc(memory_size);
    newProcess->memory_limit = memory_size;
    newProcess->num_of_threads = 0;
}

// Kill a process by PID
void killProcess(uint32_t processID)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    Process *p = &processTable[processID - 1];
    kfree((void *)p->memory_base, p->memory_limit); // Free allocated memory
    p->pid = 0;                                     // Mark the process slot as free
    p->memory_base = 0;
    p->memory_limit = 0;
    p->num_of_threads = 0;
}

// Suspend a process
void suspendProcess(uint32_t processID)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    processTable[processID - 1].s = BLOCKED;
}

// Resume a process
void resumeProcess(uint32_t processID)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    processTable[processID - 1].s = READY;
}

// Allocate memory for a process
void allocateMemory(uint32_t processID, uint32_t size)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    Process *p = &processTable[processID - 1];
    kfree((void *)p->memory_base, p->memory_limit); // Free old memory
    p->memory_base = (uint32_t)kalloc(size);
    p->memory_limit = size;
}

// Add a thread to a process
void addThread(uint32_t processID)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    Process *p = &processTable[processID - 1];
    if (p->num_of_threads >= MAX_THREADS)
    {
        return; // Maximum threads reached
    }

    p->num_of_threads++;
}

// Remove a thread from a process
void removeThread(uint32_t processID)
{
    if (processID == 0 || processID > MAX_PROCESSES || processTable[processID - 1].pid == 0)
    {
        return; // Invalid process ID
    }

    Process *p = &processTable[processID - 1];
    if (p->num_of_threads == 0)
    {
        return; // No threads to remove
    }

    p->num_of_threads--;
}
