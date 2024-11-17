#include "print.h"
#include "idt.h"
#include "ahci.h"
#include "std.h"
#define uint32_t unsigned int
#define PAGE_PRESENT  (1 << 0)  // Bit 0
#define PAGE_RW       (1 << 1)  // Bit 1
#define PAGE_USER     (1 << 2)  // Bit 2
#define PAGE_ACCESSED (1 << 5)  // Bit 5
#define PAGE_DIRTY    (1 << 6)  // Bit 6
typedef struct {
    uint32_t present    : 1;  
    uint32_t rw         : 1;  
    uint32_t user       : 1;  
    uint32_t reserved   : 2;  
    uint32_t accessed   : 1;  
    uint32_t dirty      : 1;  
    uint32_t reserved2  : 2;  
    uint32_t avail      : 3;  
    uint32_t frame_addr : 20; 
} page_table_entry_t;


typedef struct {
    uint32_t present    : 1;  
    uint32_t rw         : 1;  
    uint32_t user       : 1;  
    uint32_t reserved   : 2;  
    uint32_t accessed   : 1;  
    uint32_t reserved2  : 1; 
    uint32_t page_size  : 1;  
    uint32_t global     : 1;  
    uint32_t avail      : 3;  
    uint32_t table_addr : 20; 
} page_directory_entry_t;

page_table_entry_t page_table[1024] __attribute__((aligned(0x1000)));
page_table_entry_t page_table2[1024] __attribute__((aligned(0x1000)));
page_directory_entry_t page_directory[1024] __attribute__((aligned(0x1000)));
void setup_identity_mapping();
void map_page(void *physaddr, void *virtualaddr, unsigned int flags);
int _start()
{
    setup_identity_mapping();
     asm volatile("mov %0, %%cr3" :: "r"(page_directory)); 
     uint32_t cr0;
   
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    
    cr0 |= 0x80000000;
    
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    
    map_page(0xB8000, 0x30000,0);
    init_idt();
    load_idt();
    uint32_t* ahci_add = find_ahci_controller();
    if (!ahci_add)
    {
    char* err = "couldnt find ahci\n";
    print(err);
    }
    map_page(ahci_add,0x20000,0);
    HBA_MEM* abar = (HBA_MEM*)(0x20000);
    
    probe_port(abar);
   
    char* port1 = (char*)(abar) + 0x100;
    HBA_PORT* port = (HBA_PORT*)(port1);
    char num[32];	// Issue command
	
    port_rebase(port,0);
    char maygodbewithme[512];
    read(port,15,0,1,(uint16_t*)maygodbewithme);
    write(port,17,0,1,(uint16_t*)maygodbewithme);
    int j;
    for(j = 0;j<512;j++)
    {
        char t[1];
        t[0] = maygodbewithme[j]; //he was :)
        print(t);

    }
    
    
    
    
    while(1){}
    return 0;
}
void setup_identity_mapping() {
    
    int i;
    for (i = 0; i < 1024; i++) {
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
    for (j = 0; j < 1024; j++) {
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
}
void map_page(void *physaddr, void *virtualaddr, unsigned int flags) {
    

    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;

    page_table[ptindex].present = 1;
    page_table[ptindex].frame_addr = (unsigned long)physaddr >> 12;
    page_table[ptindex].rw = 1;
    page_table[ptindex].accessed = 1;
    page_table[ptindex].user = 0;

}