#include "std.h"
#include "gdt.h"

static gdt_entry_bits gdt[6];
static inline void load_gdt(gdt_ptr *gdt_desc)
{
    asm volatile("lgdt (%0)" ::"r"(gdt_desc));
}
uint32_t get_esp()
{
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    return esp;
}
void flush_tss()
{
    __asm__ volatile(
        "ltr %w0" // Load Task Register with the 16-bit selector
        :
        : "r"((uint16_t)((5 * 8) | 0)) // Ensure a 16-bit operand
        : "memory");
}

void jump_usermode(void *user_function, page_directory_entry_t *pd, uint32_t user_stack)
{ // Example user-mode stack

    __asm__ volatile( // Load page directory (must be before segment change)

        "mov %1, %%ax\n"   // Move Ring 3 data selector into AX
        "mov %%ax, %%ds\n" // Set DS register to the data segment selector
        "mov %%ax, %%es\n" // Set ES register
        "mov %%ax, %%fs\n" // Set FS register
        "mov %%ax, %%gs\n" // Set GS register

        "push %4\n" // Push Ring 3 SS
        "push %5\n" // Push Ring 3 ESP
        "pushf\n"   // Push EFLAGS
        "pop %%eax\n"
        "or $0x200, %%eax\n" // Set Interrupt Flag (IF)
        "push %%eax\n"
        "push %2\n" // Push Ring 3 CS
        "push %3\n"
        "sti\n"  // Push user function address (entry point)
        "iret\n" // Perform the far jump to user function
        :
        : "r"(pd),                       // Page directory (address of PD)
          "rm"((uint16_t)((4 * 8) | 3)), // Ring 3 data segment selector
          "rm"((uint16_t)((3 * 8) | 3)), // Ring 3 code segment selector
          "r"(user_function),            // User-mode function
          "r"((uint32_t)((4 * 8) | 3)),  // Ring 3 SS
          "r"(user_stack)                // Ring 3 ESP
        : "ax", "eax");
}

void setup_gdt()
{
    gdt_entry_bits *nl_seg = &gdt[0];
    gdt_entry_bits *ker_code = &gdt[1];
    gdt_entry_bits *ker_data = &gdt[2];
    gdt_entry_bits *ring3_code = &gdt[3];
    gdt_entry_bits *ring3_data = &gdt[4];

    nl_seg->limit_low = 0x0000;
    nl_seg->base_low = 0x000000;
    nl_seg->accessed = 0;
    nl_seg->read_write = 0;             // Not used, since this is a null segment
    nl_seg->conforming_expand_down = 0; // Not used
    nl_seg->code = 0;                   // Not used
    nl_seg->code_data_segment = 0;      // Not used
    nl_seg->DPL = 0;                    // Not used
    nl_seg->present = 0;                // Not used
    nl_seg->limit_high = 0x0;           // Not used
    nl_seg->available = 0;              // Not used
    nl_seg->long_mode = 0;              // Not used
    nl_seg->big = 0;                    // Not used
    nl_seg->gran = 0;                   // Not used
    nl_seg->base_high = 0x00;           // Not used

    ker_code->limit_low = 0xFFFF;  // 64KB limit
    ker_code->base_low = 0x000000; // Base address
    ker_code->accessed = 0;
    ker_code->read_write = 0;             // Code segment, so it's not writable
    ker_code->conforming_expand_down = 0; // Code segment doesn't expand down
    ker_code->code = 1;                   // It's a code segment
    ker_code->code_data_segment = 1;      // It's a code/data segment, not a TSS or LDT
    ker_code->DPL = 0;                    // Privilege level 0 (kernel)
    ker_code->present = 1;                // Present
    ker_code->limit_high = 0xF;           // 4GB limit, because the limit is split across two 16-bit fields
    ker_code->available = 0;
    ker_code->long_mode = 0; // Not long mode (64-bit)
    ker_code->big = 1;       // 32-bit code (use 32-bit instructions)
    ker_code->gran = 1;      // Use 4KB pages
    ker_code->base_high = 0x00;

    *ker_data = *ker_code;
    ker_data->read_write = 1;
    ker_data->code = 0;

    ring3_code->limit_low = 0xFFFF;
    ring3_code->base_low = 0;
    ring3_code->accessed = 0;
    ring3_code->read_write = 1;             // since this is a code segment, specifies that the segment is readable
    ring3_code->conforming_expand_down = 0; // does not matter for ring 3 as no lower privilege level exists
    ring3_code->code = 1;
    ring3_code->code_data_segment = 1;
    ring3_code->DPL = 3; // ring 3
    ring3_code->present = 1;
    ring3_code->limit_high = 0xF;
    ring3_code->available = 1;
    ring3_code->long_mode = 0;
    ring3_code->big = 1;  // it's 32 bits
    ring3_code->gran = 1; // 4KB page addressing
    ring3_code->base_high = 0;

    *ring3_data = *ring3_code; // contents are similar so save time by copying
    ring3_data->code = 0;

    gdt_ptr desc;
    desc.size = sizeof(gdt) - 1;
    desc.base = (uint32_t)&gdt;
    load_gdt(&desc);

    write_tss(&gdt[5]);

    flush_tss();
}
tss_entry_t tss_entry;
void write_tss(gdt_entry_bits *g)
{
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry);

    // Add a TSS descriptor to the GDT.
    g->limit_low = limit;
    g->base_low = base;
    g->accessed = 1;               // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
    g->read_write = 0;             // For a TSS, indicates busy (1) or not busy (0).
    g->conforming_expand_down = 0; // always 0 for TSS
    g->code = 1;                   // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
    g->code_data_segment = 0;      // indicates TSS/LDT (see also `accessed`)
    g->DPL = 0;
    g->present = 1;
    g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
    g->available = 0;                            // 0 for a TSS
    g->long_mode = 0;
    g->big = 0;                                 // should leave zero according to manuals.
    g->gran = 0;                                // limit is in bytes, not pages
    g->base_high = (base & (0xff << 24)) >> 24; // isolate top byte

    // Ensure the TSS is initially zero'd.
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = (2 << 3); // Set the kernel stack segment.
    tss_entry.esp0 = get_esp();
}
void set_kernel_stack(uint32_t stack)
{ // Used when an interrupt occurs
    tss_entry.esp0 = stack;
}