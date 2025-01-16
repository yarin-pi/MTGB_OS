#include "vm.h"

page_table_entry_t page_table[1024] __attribute__((aligned(0x1000)));

page_table_entry_t page_table2[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table3[1024] __attribute__((aligned(0x1000)));
page_directory_entry_t page_directory[1024] __attribute__((aligned(0x1000)));

void *setup_identity_mapping()
{
    for (int i = 0; i < 1024; i++) {
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
    for (int i = 0; i < 1024; i++) {
        page_table2[i].present = 1;
        page_table2[i].rw = 1;
        page_table2[i].user = 0;
        page_table2[i].frame_addr = i ; // Offset: 256 * 4KB = 1MB (0x00100000)
    }
    page_directory[768].present = 1; // 768 * 4MB = 3GB (0xC0000000)
    page_directory[768].rw = 1;
    page_directory[768].user = 0;
    page_directory[768].table_addr = ((uint32_t)page_table2) >> 12;

    for (int i = 0; i < 1024; i++) {
        page_table3[i].present = 1;
        page_table3[i].rw = 1;
        page_table3[i].user = 0;
        page_table3[i].frame_addr = (i + (0x400000 / 0x1000)); 
    }
    page_directory[1].present = 1;
    page_directory[1].rw = 1;
    page_directory[1].user = 0;
    page_directory[1].table_addr = ((uint32_t)page_table3) >> 12;
    return (void *)page_directory;
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags)
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    page_table2[ptindex].present = 1;
    page_table2[ptindex].frame_addr = (unsigned long)physaddr >> 12;
    page_table2[ptindex].rw = 1;
    page_table2[ptindex].accessed = 1;
    page_table2[ptindex].user = 0;
}
uint32_t* virt_to_phys(void* virtual)
{
    int pdi_index = (unsigned long)virtual >> 22;
    int pti_index = (unsigned long)virtual >> 12 & 0x3ff;
    int offset = (unsigned long)virtual & 0xfff;
    page_table_entry_t* ent = page_directory[pdi_index].table_addr << 12;
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