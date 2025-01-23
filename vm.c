#include "vm.h"

page_table_entry_t page_table[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table2[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table3[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table4[1024] __attribute__((aligned(0x1000)));
page_directory_entry_t page_directory[1024] __attribute__((aligned(0x1000)));

Buddy bud;

void *setup_identity_mapping()
{
    for (int i = 0; i < 1024; i++)
    {
        page_table[i].present = 1;
        page_table[i].rw = 1;
        page_table[i].user = 0;
        page_table[i].frame_addr = i; // Identity mapping
    }
    page_directory[0].present = 1;
    page_directory[0].rw = 1;
    page_directory[0].user = 0;
    page_directory[0].table_addr = ((uint32_t)page_table) >> 12;

    // Map higher-half kernel (0xC0000000 -> 0x00100000)
    for (int i = 0; i < 1024; i++)
    {
        page_table2[i].present = 1;
        page_table2[i].rw = 1;
        page_table2[i].user = 0;
        page_table2[i].frame_addr = i; // Offset: 256 * 4KB = 1MB (0x00100000)
    }
    page_directory[768].present = 1; // 768 * 4MB = 3GB (0xC0000000)
    page_directory[768].rw = 1;
    page_directory[768].user = 0;
    page_directory[768].table_addr = ((uint32_t)page_table2) >> 12;

    for (int i = 0; i < 1024; i++)
    {
        page_table3[i].present = 1;
        page_table3[i].rw = 1;
        page_table3[i].user = 0;
        page_table3[i].frame_addr = (i + (0x400000 / 0x1000));
    }
    page_directory[1].present = 1;
    page_directory[1].rw = 1;
    page_directory[1].user = 0;
    page_directory[1].table_addr = ((uint32_t)page_table3) >> 12;

    for (int i = 0; i < 1024; i++)
    {
        page_table4[i].present = 1;
        page_table4[i].rw = 1;
        page_table4[i].user = 0;
        page_table4[i].frame_addr = (i + (0x800000 / 0x1000));
    }

    page_directory[769].present = 1;
    page_directory[769].rw = 1;
    page_directory[769].user = 0;
    page_directory[769].table_addr = ((uint32_t)page_table4) >> 12;
    return (void *)page_directory;
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags, page_table_entry_t *page_table)
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    page_table[ptindex].present = 1;
    page_table[ptindex].frame_addr = (unsigned long)physaddr >> 12;
    page_table[ptindex].rw = 1;
    page_table[ptindex].accessed = 1;
    page_table[ptindex].user = 0;
}
void unmap_page(void *virtual_address, page_table_entry_t *page_table)
{
    uint32_t vaddr = (uint32_t)virtual_address;

    // Calculate page directory and page table indices // Top 10 bits
    uint32_t pt_index = (vaddr >> 12) & 0x3FF; // Next 10 bits

    // Get the page table entry from the page directory
    if (!page_table)
    {
        // Page table does not exist, nothing to unmap
        return;
    }

    // Remove the mapping in the page table
    page_table[pt_index].present = 0;

    // Invalidate the TLB for the given virtual address
    asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");
}
uint32_t *virt_to_phys(void *virtual)
{
    int pdi_index = (unsigned long)virtual >> 22;
    int pti_index = (unsigned long)virtual >> 12 & 0x3ff;
    int offset = (unsigned long)virtual & 0xfff;
    page_table_entry_t *ent = page_directory[pdi_index].table_addr << 12;
    ent = HIGHER_HALF(ent);
    return ((ent[pti_index].frame_addr << 12) | offset);
}

void page_fault_handler(uint32_t error_code)
{
    uint32_t faulting_address;

    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    int present = !(error_code & 0x1);
    int write = error_code & 0x2;
    int user = error_code & 0x4;
    int reserved = error_code & 0x8;
    int instruction_fetch = error_code & 0x10;

    if (present)
    {
        void *new_page; // should malloc bruh

        if (new_page == 0)
        {
            print("no physical memory left to allocate\n");
        }
    }
    else if (write)
    {
        print("Tried to change read-only file fault\n");
    }
    else if (user)
    {
        print("userspace process cannot operate outside of premitted memory regions fault\n");
    }
    else if (reserved)
    {
        print("reserved bits overwritten fault\n");
    }
    else if (instruction_fetch)
    {
        print("attempt to fetch an instruction from a non-executable page fault\n");
    }
    else
    {
        print("unknown page fault error code\n");
    }
}
void init_recursivePage()
{
    page_directory[1023].table_addr = page_directory;
}

void init_kalloc()
{
    bud.base_address = HEAP_ADDR;
    bud.total_size = HEAP_SIZE;
    bud.max_order = 3;
    init_buddy(&bud);
}
void *kalloc(uint32_t size)
{
    if (size > (1 << 15))
    {
        print("size is above 32k \n");
        return 0;
    }

    void *phys_ptr = balloc(&bud, size);
    map_page(phys_ptr, phys_ptr, 0, page_table4);

    return phys_ptr;
}

void kfree(void *addr, uint32_t size)
{
    if (!addr)
    {
        return;
    }

    unmap_page(addr, page_table4);
    bfree(&bud, addr, size >> 12);
}
