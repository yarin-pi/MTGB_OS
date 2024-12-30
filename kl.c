#include "print.h"
#include "idt.h"
#include "ahci.h"
#include "std.h"
#include "fs.h"
#include "vm.h"
#include "keyboard.h"
#include "clock.h"


#define HIGHER_HALF(addr) ((void*)((uint32_t)(addr) + 0xC0000000))

void abc();
int _start()
{
    void *page_directory = setup_identity_mapping();
    
    asm volatile("mov %0, %%cr3" ::"r"(page_directory));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000;

    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    abc();
    map_page(0xB8000, 0xC0150000, 0);
    init_idt();
    load_idt();

    uint32_t *ahci_add = find_ahci_controller();
    if (!ahci_add)
    {
        char *err = "couldnt find ahci\n";
        print(err);
    }
    map_page(ahci_add, 0xc0120000, 0);
    HBA_MEM *abar = (HBA_MEM *)(0xc0120000);

    probe_port(abar);
    abar = (HBA_MEM *)(0xc0120000);
    char *port1 = (char *)(abar) + 0x100;
    HBA_PORT *port = (HBA_PORT *)(port1);
    char num[32]; // Issue command
    

    port_rebase(port, 0);
    for (int i = 0; i < IDT_SIZE; i++)
    {
        
        set_idt_entry(i, (uint32_t)HIGHER_HALF(unhandled_interrupt_handler), 0x08, 0x8E);
        
    }
    
    set_idt_entry(14, (uint32_t)HIGHER_HALF(page_fault_handler), 0x08, 0x8e);

    set_idt_entry(
        33,                         // Vector number for keyboard IRQ1
        (uint32_t)HIGHER_HALF(keyboard_handler), // Address of the handler
        0x08,                       // Code segment selector
        0x8E                        // Type attribute: present, privilege 0, 32-bit interrupt gate
    );
    
    

    asm volatile("sti");
    clear_screen();
    enable_keyboard_interrupt();
    print("test buddy: \n"); 
    Buddy bud;
    bud.base_address = 0x200000;
    bud.max_order = 3;
    bud.total_size = 0x1000000;

    init_buddy(&bud);
    int i = 5;
    
    void* ptr = balloc(&bud, 0x2000);
    char* no[32];
    int_to_string((uint32_t)ptr,no,16);
    print(no);
    print("\n");
    bfree(&bud, ptr,1);
    ptr = balloc(&bud, 0x2000);
   
    int_to_string((uint32_t)ptr,no,16);
    print(no);
    print("\n");
   
    
   
    




    while (1)
    {
    }
    return 0;
}
void abc()
{
    

    
    asm volatile("pop %eax");  
    asm volatile("pop %eax");  
    asm volatile("add $0xc0000000, %eax");
    asm volatile("add $0xc0200000, %esp");
    asm volatile("jmp *%eax"); 

}