#include "keyboard.h"

void keyboard_handler()
{
    uint8_t scancode = inl(KEYBOARD_DATA_PORT);

    uint8_t c = scancode_to_char(scancode);
    if (c != 0)
    {
        print_char(c);
    }

    outl(PIC1_COMMAND, PIC_EOI);
}

uint8_t scancode_to_char(uint8_t scancode)
{
    static char keymap[128] = {
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
    uint8_t mask = inl(PIC1_DATA);
    mask &= ~(1 << 1); // Clear bit 1 to enable IRQ1
    outl(PIC1_DATA, mask);
}