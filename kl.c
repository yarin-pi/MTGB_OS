#include "print.h"
#include "idt.h"
#include "ahci.h"
#include "std.h"
#include "fs.h"
#include "vm.h"
#include "keyboard.h"
#define uint32_t unsigned int

int _start()
{
    void *page_directory = setup_identity_mapping();
    asm volatile("mov %0, %%cr3" ::"r"(page_directory));
    uint32_t cr0;

    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000;

    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    map_page(0xB8000, 0x30000, 0);
    init_idt();
    load_idt();
    uint32_t *ahci_add = find_ahci_controller();
    if (!ahci_add)
    {
        char *err = "couldnt find ahci\n";
        print(err);
    }
    map_page(ahci_add, 0x20000, 0);
    HBA_MEM *abar = (HBA_MEM *)(0x20000);

    probe_port(abar);

    char *port1 = (char *)(abar) + 0x100;
    HBA_PORT *port = (HBA_PORT *)(port1);
    char num[32]; // Issue command

    port_rebase(port, 0);
    FatInitImage(port);
    FatAddFile("/hi.txt", "messi better", 18);

    enable_keyboard_interrupt();
    asm volatile("sti"); // Enable interrupts globally

    while (1)
    {
    }
    return 0;
}
