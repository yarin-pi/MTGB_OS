#ifndef VM_H
#define VM_H
#define PAGE_PRESENT (1 << 0)  // Bit 0
#define PAGE_RW (1 << 1)       // Bit 1
#define PAGE_USER (1 << 2)     // Bit 2
#define PAGE_ACCESSED (1 << 5) // Bit 5
#define PAGE_DIRTY (1 << 6)    // Bit 6
#define PF_X 1
#define PF_W 2 
#define PF_R 4
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#include "pm.h"
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

typedef struct
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} ProgramHeader;
#define HEAP_ADDR 0xc0400000
#define HEAP_SIZE 0xf0000000 - 0xc0400000
void *setup_identity_mapping();
void map_page(void *physaddr, void *virtualaddr, unsigned int flags, page_table_entry_t *page_table);
void unmap_page(void *virtual_address, page_table_entry_t *page_table);
void page_fault_handler(uint32_t error_code);
void init_recursivePage();
void *kalloc(uint32_t size);
void kfree(void *addr, uint32_t size);
uint32_t *virt_to_phys(void *virtual);
void init_kalloc();
void enb_4mb();
void init_palloc();
void palloc(ProgramHeader* ph, void* f_addr, page_directory_entry_t* pd);
page_directory_entry_t* create_page_directory();
void switch_page_directory(uint32_t* new_pd);
extern Buddy bud;
extern Buddy pbud;
extern page_table_entry_t page_table[1024];
extern page_table_entry_t page_table2[1024];
extern page_table_entry_t page_table3[1024];
extern page_table_entry_t page_table4[1024];
extern page_directory_entry_t page_directory[1024];
#endif VM_H // DEBUG