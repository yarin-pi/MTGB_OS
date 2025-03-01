#ifndef PM_H
#define PM_H
#include "std.h"
#define MAX_SIZE 1 << 15
#define MIN_SIZE 1 << 12
#define MAX_ORDER 3

typedef struct
{
    void *base_address;
    uint32_t total_size;
    int max_order;
    uint8_t *bitmap[4];

} Buddy;

void init_buddy(Buddy* buddy,uint8_t index);
void* balloc(Buddy* buddy,uint32_t alloc_size,uint8_t bitmap_index);
void bfree(Buddy* buddy,void* ptr, uint16_t order,uint8_t bitmap_index);
#endif PM_H
