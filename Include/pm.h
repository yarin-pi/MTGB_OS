#ifndef PM_H
#define PM_H 
#include "std.h"
#define MAX_SIZE 1 << 15
#define MIN_SIZE 1 << 12
#define MAX_ORDER 3

typedef struct 
{
    void* base_address;
    uint32_t total_size;
    int max_order;
    uint8_t* bitmap[4];
   
} Buddy;

Buddy* init_buddy(Buddy* buddy);


#endif PM_H

