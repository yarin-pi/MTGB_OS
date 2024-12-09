#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "print.h"
#include "std.h"
#define KEYBOARD_DATA_PORT 0x60
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC_EOI 0x20

// IRQ1 vector
#define IRQ1_VECTOR 0x21

void keyboard_handler();                    // when key is pressed function is called to handle the pressed event
uint8_t scancode_to_char(uint8_t scancode); // translate the pressed key to a character
void enable_keyboard_interrupt();           // enable the option to recive the keyboard press interrupt

#endif KEYBOARD_H