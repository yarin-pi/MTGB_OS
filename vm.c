
#include "vm.h"
#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_4MB 0x80
page_table_entry_t page_table[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table2[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table3[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table4[1024] __attribute__((aligned(0x1000)));
page_directory_entry_t page_directory[1024] __attribute__((aligned(0x1000)));
Buddy bud;
Buddy pbud;
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
    return (void *)page_directory;
}
void switch_page_directory(uint32_t *new_pd)
{
    asm volatile("mov %0, %%cr3" ::"r"(new_pd));
}
void enb_4mb()
{
    uint32_t base_addr = 0x00800000; // Start from 8MB
    for (int i = 0; i < 199; i++)    // 199 * 4MB = 796MB
    {
        page_directory[769 + i].present = 1;
        page_directory[769 + i].rw = 1;
        page_directory[769 + i].user = 0;
        page_directory[769 + i].page_size = 1;                  // Set PS bit for 4MB pages
        page_directory[769 + i].table_addr = (base_addr >> 12); // Correct shift
        base_addr += 0x400000;                                  // Move to next 4MB region
    }
}
void map_page(void *physaddr, void *virtualaddr, unsigned int flags, page_table_entry_t *page_table)
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
    if (flags == 1)
    {
        page_directory[pdindex].present = 1;
        page_directory[pdindex].table_addr = (unsigned long)physaddr >> 12;
        page_directory[pdindex].rw = 1;
        page_directory[pdindex].user = 0;
        page_directory[pdindex].page_size = 1;
    }
    else
    {
        page_table[ptindex].present = 1;
        page_table[ptindex].frame_addr = (unsigned long)physaddr >> 12;
        page_table[ptindex].rw = 1;
        page_table[ptindex].accessed = 1;
        page_table[ptindex].user = 0;
    }
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
void set_present(void *virtual_address, page_table_entry_t *page_table)
{
    uint32_t vaddr = (uint32_t)virtual_address;
    uint32_t pt_index = (vaddr >> 12) & 0x3FF; // Next 10 bits
    if (!page_table)
    {

        return;
    }

    page_table[pt_index].present = 1;
}
uint32_t *virt_to_phys(void *virtual)
{
    int pdi_index = (unsigned long)virtual >> 22;
    int pti_index = (unsigned long)virtual >> 12 & 0x3ff;
    int offset = (unsigned long)virtual & 0xfff;
    if (page_directory[pdi_index].page_size)
    {
        offset = (unsigned int)virtual & 0x3fffff;
        return (page_directory[pdi_index].table_addr << 12) | offset;
    }
    page_table_entry_t *ent = page_directory[pdi_index].table_addr << 12;
    ent = HIGHER_HALF(ent);
    return ((ent[pti_index].frame_addr << 12) | offset);
}
page_directory_entry_t *create_page_directory()
{
    page_directory_entry_t *new_pd = (uint32_t *)kalloc(4096);
    memset(new_pd, 0, 4096);

    // Copy kernel mappings (last 256 entries)
    new_pd[0] = page_directory[0];
    new_pd[1] = page_directory[1];
    for (int i = 768; i < 1024; i++)
    {
        new_pd[i] = page_directory[i];
    }

    return new_pd;
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
    page_directory[1023].present = 1;
    page_directory[1023].rw = 1;
    page_directory[1023].user = 0;
}

void init_kalloc()
{
    bud.base_address = HEAP_ADDR;
    bud.total_size = HEAP_SIZE;
    bud.max_order = 3;
    init_buddy(&bud, 0);
}

void *kalloc(uint32_t size)
{
    if (size > (1 << 15))
    {
        print("size is above 32k \n");
        return 0;
    }

    void *phys_ptr = balloc(&bud, size, 0);
    set_present(phys_ptr, page_table4);

    return phys_ptr;
}

void kfree(void *addr, uint32_t size)
{
    if (!addr)
    {
        return;
    }

    unmap_page(addr, page_table4);
    bfree(&bud, addr, size >> 12, 0);
}

void init_palloc()
{
    pbud.base_address = 0x60000000;
    pbud.total_size = 0xc0000000 - 0x60000000;
    pbud.max_order = 3;
    init_buddy(&pbud, 1);
}

page_table_entry_t *tmp;
void palloc(ProgramHeader *ph, void *f_addr, page_directory_entry_t *pd)
{
    char *adr = (char *)f_addr + ph->p_offset;
    unsigned long pdindex = (unsigned long)ph->p_vaddr >> 22;
    unsigned long ptindex = (unsigned long)ph->p_vaddr >> 12 & 0x03FF;

    // Check if Page Table exists
    page_table_entry_t *Page;
    if (!pd[pdindex].present)
    {
        Page = (page_table_entry_t *)kalloc(4096);
        tmp = Page;
        memset(Page, 0, 4096); // Clear new page table
        pd[pdindex].rw = (ph->p_flags & PF_W) ? 1 : 0;
        pd[pdindex].present = 1;
        pd[pdindex].user = 1;
        pd[pdindex].table_addr = (uint32_t)virt_to_phys(Page) >> 12;
    }
    else
    {
        Page = tmp;
    }

    // Allocate frames and map memory
    for (int i = 0; i < ph->p_memsz; i += 4096)
    {
        uint32_t inx = i / 4096;
        uint32_t n_ptIn = ptindex + inx;

        Page[n_ptIn].rw = (ph->p_flags & PF_W) ? 1 : 0;
        Page[n_ptIn].user = 1;
        Page[n_ptIn].present = 1;
        Page[n_ptIn].frame_addr = (uint32_t)balloc(&pbud, 4096, 1) >> 12;

        memcpy(ph->p_vaddr, (void *)adr, ph->p_filesz);

        // Zero out remaining memory if memsz > filesz
    }
}
