#include "mem.h"
#include "std.h"

#define MEMORY_BASE 0x100000  // Starting address of managed memory
#define MEMORY_SIZE (1 << 31) // 2 GB total memory
#define MIN_BLOCK_SIZE 1024   // Minimum block size (e.g., 1 KB)
#define MAX_LEVEL 10

// Define a simple linked list for the free list
typedef struct free_block
{
    uint32_t address;
    struct free_block *next;
} free_block_t;

static free_block_t *free_list[MAX_LEVEL];

// Initialize the buddy allocator
void init_buddy()
{
    for (uint32_t i = 0; i < MAX_LEVEL; i++)
    {
        free_list[i] = 0;
    }

    // Start with a single large free block
    free_block_t *initial_block = (free_block_t *)MEMORY_BASE;
    initial_block->address = MEMORY_BASE;
    initial_block->next = 0;
    free_list[MAX_LEVEL - 1] = initial_block;
}

// Get the level of a block for the given size
uint32_t get_block_level(uint32_t size)
{
    uint32_t level = 0;
    uint32_t block_size = MIN_BLOCK_SIZE;

    while (block_size < size && level < MAX_LEVEL)
    {
        block_size *= 2;
        level++;
    }
    return level;
}

// Split a block at the given level
void split_block(uint32_t level)
{
    if (!free_list[level])
        return;

    // Remove a block from the current level
    free_block_t *block = free_list[level];
    free_list[level] = block->next;

    // Calculate buddy addresses
    uint32_t buddy1 = block->address;
    uint32_t buddy2 = buddy1 + (1 << (level - 1)) * MIN_BLOCK_SIZE;

    // Add the two buddies to the next smaller level
    free_block_t *new_block1 = (free_block_t *)buddy1;
    new_block1->address = buddy1;
    new_block1->next = free_list[level - 1];

    free_block_t *new_block2 = (free_block_t *)buddy2;
    new_block2->address = buddy2;
    new_block2->next = new_block1;

    free_list[level - 1] = new_block2;
}

// Allocate memory
void *malloc(uint32_t size)
{
    uint32_t level = get_block_level(size);

    // Find a free block at the desired level or higher
    uint32_t alloc_level = level;
    while (alloc_level < MAX_LEVEL && !free_list[alloc_level])
    {
        alloc_level++;
    }

    if (alloc_level == MAX_LEVEL)
    {
        return 0; // No free block found
    }

    // Split blocks until the desired level is reached
    while (alloc_level > level)
    {
        split_block(alloc_level--);
    }

    // Allocate the block
    free_block_t *block = free_list[level];
    free_list[level] = block->next;

    return (void *)block->address;
}

// Free a memory block
void free(void *ptr)
{
    uint32_t block_addr = (uint32_t)ptr;
    uint32_t level = 0;

    // Determine the level of the block
    uint32_t block_size = MIN_BLOCK_SIZE;
    while (block_size < MEMORY_SIZE && (block_addr & block_size))
    {
        level++;
        block_size *= 2;
    }

    // Add the block back to the free list
    free_block_t *block = (free_block_t *)block_addr;
    block->address = block_addr;
    block->next = free_list[level];
    free_list[level] = block;

    // Check for buddy merging
    uint32_t buddy_addr = block_addr ^ ((1 << level) * MIN_BLOCK_SIZE);
    free_block_t *buddy = free_list[level];

    // Traverse the free list to find the buddy
    free_block_t **prev = &free_list[level];
    while (buddy)
    {
        if (buddy->address == buddy_addr)
        {
            // Merge buddies
            *prev = buddy->next; // Remove buddy from free list
            uint32_t parent_addr = block_addr & ~((1 << level) * MIN_BLOCK_SIZE);
            free(parent_addr); // Recursively merge upward
            return;
        }
        prev = &buddy->next;
        buddy = buddy->next;
    }
}
