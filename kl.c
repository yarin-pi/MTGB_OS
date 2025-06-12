#include "print.h"
#include "idt.h"
#include "ahci.h"
#include "std.h"
#include "fs.h"
#include "vm.h"
#include "keyboard.h"
#include "clock.h"
#include "exe.h"
#include "syscall.h"
#include "vesa.h"
int _start()
{

    void *page_directory = setup_identity_mapping();

    asm volatile("mov %0, %%cr3" ::"r"(page_directory));
    asm volatile(
        "mov %%cr4, %%eax\n"
        "or $0x10, %%eax\n" // Set PSE (bit 4) to enable 4MB pages
        "mov %%eax, %%cr4\n" ::: "eax", "cc");

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000;

    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    enb_4mb();
    MoveHigherHalf();
    map_page(0xB8000, 0xC0150000, 0, page_table2);

    init_idt();
    load_idt();
    init_framebuffer();

    uint32_t *ahci_add = find_ahci_controller();
    if (!ahci_add)
    {
        char *err = "couldnt find ahci\n";
        print(err);
    }
    map_page(ahci_add, 0xc0120000, 0, page_table2);
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
        33,                                      // Vector number for keyboard IRQ1
        (uint32_t)HIGHER_HALF(keyboard_handler), // Address of the handler
        0x08,                                    // Code segment selector
        0x8E                                     // Type attribute: present, privilege 0, 32-bit interrupt gate
    );
    set_idt_entry(0x80, (uint32_t)HIGHER_HALF(handle_syscall), 0x08, 0xEE);

    setup_gdt();
    asm volatile("sti");
    print("hello");
    FatInitImage(port);
    init_kalloc();
    char *arrxe = (char *)kalloc(0x24a8);
    getContent("/simple.elf", (void *)arrxe);
    enable_keyboard_interrupt();
    init_palloc();
    clock_init();
    init_framebuffer();
    clear_screen();
    vprint("Welcome to MTGB operating-system!");
    vprint("\n");
    // elf_load_file(arrxe);
    map_page(0xD0000000, 0x20000, 0, page_table);
    vprint("mapping page:\nphysical address: 0xD0000000\nvirtual address: 0x20000");
    vprint("checking if it worked: ");
    vprint("the physical address of 0x20000 is:\n");
    vprint_int(virt_to_phys(0x20000), 16);
    
    return 0;
}
void MoveHigherHalf()
{
    asm volatile(
        "pop %%eax\n"
        "pop %%eax\n"
        "add $0xc0000000, %%eax\n"
        "add $0xf0000000, %%esp\n"
        "jmp *%%eax"
        :
        :
        : "eax", "esp");
}
