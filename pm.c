#include "pm.h"
#define POOL_S 1966080 / 8
static uint8_t bitm_pool[2][POOL_S]; // Two bitmaps

void init_buddy(Buddy *buddy, uint8_t index)
{
    int offset = 0;
    for (int i = 0; i <= buddy->max_order; i++)
    {
        buddy->bitmap[i] = bitm_pool[index] + offset;
        for (int j = 0; j < buddy->total_size / ((1 << i) * 4096); j++)
        {
            if (i == buddy->max_order)
            {
                buddy->bitmap[i][j / 8] &= ~(1 << (j % 8));
            }
            else
            {
                buddy->bitmap[i][j / 8] |= 1 << (j % 8);
            }
        }
        offset += ((buddy->total_size / ((1 << i) * 4096)) / 8);
    }
}

uint16_t get_level(uint32_t alloc_size)
{
    uint16_t level = 0;
    uint32_t block_size = MIN_SIZE;
    while (level < MAX_ORDER && block_size < alloc_size)
    {
        block_size <<= 1;
        level++;
    }
    return level;
}

uint32_t split_block(Buddy *buddy, uint16_t level, uint32_t index, uint16_t depth, uint8_t bitmap_index)
{
    uint8_t *map = buddy->bitmap[level];
    map[index / 8] |= 1 << (index % 8);
    if (!depth)
    {
        return index;
    }

    map = buddy->bitmap[level - 1];
    map[(index * 2) / 8] |= (1 << (index * 2 % 8));
    map[(index * 2) / 8] &= ~(1 << ((index * 2 + 1) % 8));
    return split_block(buddy, level - 1, index * 2, depth - 1, bitmap_index);
}

void merge_block(Buddy *buddy, uint16_t level, uint32_t index, uint8_t bitmap_index)
{
    uint8_t *map = buddy->bitmap[level];
    if ((((map[index / 8] >> (index % 8)) & 1) || ((map[index / 8] >> ((index + 1) % 8)) & 1)) || level > 3)
    {
        return;
    }
    map[index / 8] |= 1 << (index % 8);
    map[index / 8] |= 1 << ((index + 1) % 8);
    merge_block(buddy, level + 1, index / 2, bitmap_index);
}

void *balloc(Buddy *buddy, uint32_t alloc_size, uint8_t bitmap_index)
{
    uint16_t offset = 0;
    uint16_t block_lv = 0;
    uint16_t level = get_level(alloc_size);
    abool found = FALSE;
    for (int i = level; i <= buddy->max_order && !found; i++)
    {
        uint8_t *map = buddy->bitmap[i];
        for (int j = 0; j < (buddy->total_size / (1 << (i + 12))); j++)
        {
            if (map[j / 8] != 0xff)
            {
                uint8_t is_free = (map[j / 8] >> (j % 8)) & 1;
                if (!is_free)
                {
                    block_lv = i;
                    offset = j;
                    found = TRUE;
                    break;
                }
            }
        }
    }
    if (found)
    {
        uint32_t address = (uint32_t)buddy->base_address + (1 << ((uint32_t)level + 12)) * split_block(buddy, block_lv, offset, block_lv - level, bitmap_index);
        return (void *)address;
    }
    else
    {
        return 0;
    }
}

void bfree(Buddy *buddy, void *ptr, uint16_t order, uint8_t bitmap_index)
{
    uint32_t byte_offset = (uint32_t)ptr - (uint32_t)buddy->base_address;
    uint32_t index = byte_offset / (1 << (order + 12));
    buddy->bitmap[order][index / 8] &= ~(1 << (index % 8));
    merge_block(buddy, order, index, bitmap_index);
}
