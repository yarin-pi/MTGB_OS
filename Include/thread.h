#include "std.h"

// Thread states (similar to process states)


struct thread
{
    uint32_t tid;              // Thread ID
    uint32_t pid;              // Parent process ID
    uint32_t *stack_pointer;   // Stack pointer
    uint32_t *program_counter; // Program counter
    state t_state;             // Thread state (e.g., READY, RUNNING)
    void *context;             // CPU context (e.g., registers, execution state)
};