#ifndef IDT_H
#define IDT_H
#include "print.h"
#include "keyboard.h"
#define IDT_SIZE 256
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
// Structure for an IDT entry
typedef struct
{
    uint16_t offset_low;  // Lower 16 bits of the handler function address
    uint16_t selector;    // Kernel segment selector
    uint8_t zero;         // Always set to zero
    uint8_t type_attr;    // Type and attributes
    uint16_t offset_high; // Upper 16 bits of the handler function address
} __attribute__((packed)) idt_entry_t;

// Structure for loading the IDT with the 'lidt' instruction
typedef struct
{
    uint16_t limit; // Size of the IDT
    uint32_t base;  // Address of the first IDT entry
} __attribute__((packed)) idt_ptr_t;
void set_idt_entry(int vector, uint32_t handler, uint16_t selector, uint8_t type_attr);
void init_idt();
void unhandled_interrupt_handler(struct interrupt_frame *frame);
void load_idt();
#endif IDT_H
