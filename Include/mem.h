#ifndef MEM_H
#define MEM_H
void init_buddy();
uint32_t get_block_level(uint32_t size);
void split_block(uint32_t level);
void *malloc(uint32_t size);
void free(void *ptr);

#endif MEM_H

///