#ifndef VM_H
#define VM_H
#define PAGE_PRESENT (1 << 0)  // Bit 0
#define PAGE_RW (1 << 1)       // Bit 1
#define PAGE_USER (1 << 2)     // Bit 2
#define PAGE_ACCESSED (1 << 5) // Bit 5
#define PAGE_DIRTY (1 << 6)    // Bit 6
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef struct
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t reserved : 2;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t reserved2 : 2;
    uint32_t avail : 3;
    uint32_t frame_addr : 20;
} page_table_entry_t;

typedef struct
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t reserved : 2;
    uint32_t accessed : 1;
    uint32_t reserved2 : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t table_addr : 20;
} page_directory_entry_t;

void *setup_identity_mapping();
void map_page(void *physaddr, void *virtualaddr, unsigned int flags);
void page_fault_handler(uint32_t error_code);
void init_recursivePage();
#endif VM_H // DEBUG