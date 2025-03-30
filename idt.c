#include "idt.h"
idt_entry_t idt[IDT_SIZE];
idt_ptr_t idt_descriptor;
void set_idt_entry(int vector, uint32_t handler, uint16_t selector, uint8_t type_attr)
{
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}
// Initialize the first entry to point to write_string function
void init_idt()
{
    idt_descriptor.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idt_descriptor.base = (uint32_t)&idt;

    // Set IDT entry 0 to point to write_string function
    // 0x08 is the code segment selector, 0x8E for interrupt gate
}

__attribute__((interrupt, target("general-regs-only"))) void unhandled_interrupt_handler(struct interrupt_frame *frame)
{
    // Print an error message or halt the system
    // print("Unhandled interrupt! ");
    // print_int(frame->ip,16);
    outb(PIC1_COMMAND, PIC_EOI);
}
void load_idt()
{
    asm volatile("lidt (%0)" : : "r"(&idt_descriptor));
}
