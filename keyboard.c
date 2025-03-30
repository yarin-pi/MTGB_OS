#include "keyboard.h"

int buffer_index;

char input_buffer[BUFFER_SIZE];
__attribute__((interrupt, target("general-regs-only"))) void keyboard_handler(struct interrupt_frame *frame)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    uint8_t c = scancode_to_char(scancode);

    if (c != 0 && c != '\0') // Only valid and non-null characters
    {

        if (c == '\n')
        {
            input_buffer[buffer_index] = '\0'; // End the input buffer
            process_input(input_buffer);       // Process the buffer
            vprint_char(c);
            buffer_index = 0;
        }
        else if (buffer_index < BUFFER_SIZE - 1)
        {
            input_buffer[buffer_index++] = c; // Add character to buffer
            vprint_char(c);
        } // Print the character
    }
    outb(PIC1_COMMAND, PIC_EOI);
    if (scancode >= 0x28) // If interrupt came from slave PIC
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }
}
uint8_t scancode_to_char(uint8_t scancode)
{
    char keymap[128] = {
        0,
        27,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',
        0,
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        0,
        0,
        '\\',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        0,
        '*',
        0,
        ' ',
        0,
    };
    if (scancode < 128)
    {
        return keymap[scancode];
    }
    return 0;
}

void enable_keyboard_interrupt()
{
    outb(PIC1_COMMAND, 0x11); // Start initialization sequence (ICW1)
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20); // ICW2: Master PIC vector offset (0x20-0x27)
    outb(PIC2_DATA, 0x28); // ICW2: Slave PIC vector offset (0x28-0x2F)
    outb(PIC1_DATA, 0x04); // ICW3: Master PIC has slave at IRQ2
    outb(PIC2_DATA, 0x02); // ICW3: Slave PIC cascade identity
    outb(PIC1_DATA, 0x01); // ICW4: 8086 mode
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0xFC); // OCW1: Unmask IRQ1 (keyboard), mask all others
    outb(PIC2_DATA, 0xFF); // OCW1: Mask all IRQs on slave
    outb(PIC1_COMMAND, 0x20);
    buffer_index = 0;
}
