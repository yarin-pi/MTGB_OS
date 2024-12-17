#include "print.h"
#include "idt.h"
#include "ahci.h"
#include "std.h"
#include "fs.h"
#include "vm.h"
#include "keyboard.h"
#include "clock.h"
#define uint32_t unsigned int
int _start()
{
    void *page_directory = setup_identity_mapping();
    
    asm volatile("mov %0, %%cr3" ::"r"(page_directory));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000;

    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    map_page(0xB8000, 0xC0030000, 0);
    init_idt();
    load_idt();

    uint32_t *ahci_add = find_ahci_controller();
    if (!ahci_add)
    {
        char *err = "couldnt find ahci\n";
        print(err);
    }
    map_page(ahci_add, 0xc0020000, 0);
    HBA_MEM *abar = (HBA_MEM *)(0xc0020000);

    probe_port(abar);

    char *port1 = (char *)(abar) + 0x100;
    HBA_PORT *port = (HBA_PORT *)(port1);
    char num[32]; // Issue command
    

    port_rebase(port, 0);
    for (int i = 0; i < IDT_SIZE; i++)
    {
        
            set_idt_entry(i, (uint32_t)unhandled_interrupt_handler, 0x08, 0x8E);
        
    }
    set_idt_entry(14, (uint32_t)page_fault_handler, 0x08, 0x8e);

    set_idt_entry(
        33,                         // Vector number for keyboard IRQ1
        (uint32_t)keyboard_handler, // Address of the handler
        0x08,                       // Code segment selector
        0x8E                        // Type attribute: present, privilege 0, 32-bit interrupt gate
    );
    
    

    asm volatile("sti");
    clear_screen();
    enable_keyboard_interrupt();
    print("test");
    



    while (1)
    {
    }
    return 0;
}
