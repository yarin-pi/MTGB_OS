#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "print.h"
#include "std.h"
#include "input.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_COMMAND_PORT 0x64
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define BUFFER_SIZE 128

struct interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} __attribute__((packed));
int buffer_index = 0;
char input_buffer[BUFFER_SIZE];
void keyboard_handler(struct interrupt_frame *frame); // when key is pressed function is called to handle the pressed event
uint8_t scancode_to_char(uint8_t scancode);           // translate the pressed key to a character
void enable_keyboard_interrupt();
// enable the option to recive the keyboard press interrupt

#endif KEYBOARD_H