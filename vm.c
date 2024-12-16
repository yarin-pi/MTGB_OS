#include "vm.h"
page_table_entry_t page_table[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table2[1024] __attribute__((aligned(0x1000)));
page_directory_entry_t page_directory[1024] __attribute__((aligned(0x1000)));
void *setup_identity_mapping()
{
    int i;
    for (i = 0; i < 1024; i++)
    {
        page_table[i].present = 1;
        page_table[i].rw = 1;
        page_table[i].user = 0;
        page_table[i].frame_addr = i;
    }
    page_directory[0].present = 1;
    page_directory[0].rw = 1;
    page_directory[0].user = 0;
    page_directory[0].page_size = 0;
    page_directory[0].table_addr = ((uint32_t)page_table) >> 12;
    int j;
    for (j = 0; j < 1024; j++)
    {
        page_table2[j].present = 1;
        page_table2[j].rw = 1;
        page_table2[j].user = 0;
        page_table2[j].frame_addr = j + 1024;
    }
    page_directory[1].present = 1;
    page_directory[1].rw = 1;
    page_directory[1].user = 0;
    page_directory[1].page_size = 0;
    page_directory[1].table_addr = ((uint32_t)page_table2) >> 12;
    return (void *)page_directory;
}
void map_page(void *physaddr, void *virtualaddr, unsigned int flags)
{

    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    page_table[ptindex].present = 1;
    page_table[ptindex].frame_addr = (unsigned long)physaddr >> 12;
    page_table[ptindex].rw = 1;
    page_table[ptindex].accessed = 1;
    page_table[ptindex].user = 0;
}
void page_fault_handler(uint32_t error_code)
{
    uint32_t faulting_address;

    asm volatile("mov %%cr2, %0" : "=r"(faulting_address)); // Read faulting address from CR2

    // Log the fault details
    print("Page Fault Handler Invoked!\n");
}
