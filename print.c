#include "print.h"
#include "std.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xc0150000

// VGA color attributes (example: white text on black background)
#define DEFAULT_COLOR 0x0F

uint16_t *vga_buffer = (uint16_t *)VGA_BUFFER; // VGA buffer pointer
uint8_t cursor_x = 0;                          // Current cursor position (x)
uint8_t cursor_y = 0;                          // Current cursor position (y)
void clear_screen()
{
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga_buffer[i] = (DEFAULT_COLOR << 8) | ' '; // Blank space with default color
    }
    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

void move_cursor()
{

    uint16_t position = cursor_y * VGA_WIDTH + cursor_x;

    // Send the high byte of the cursor position
    outb(0x3D4, 14);
    outb(0x3D5, (position >> 8) & 0xFF);

    // Send the low byte of the cursor position
    outb(0x3D4, 15);
    outb(0x3D5, position & 0xFF);
}

void print_char(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (DEFAULT_COLOR << 8) | c;
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH)
    {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT)
    {
        // Scroll up if we reach the end of the screen
        int y, x;
        for (y = 1; y < VGA_HEIGHT; y++)
        {
            for (x = 0; x < VGA_WIDTH; x++)
            {
                vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
            }
        }
        for (x = 0; x < VGA_WIDTH; x++)
        {
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (DEFAULT_COLOR << 8) | ' ';
        }
        cursor_y = VGA_HEIGHT - 1;
    }
    move_cursor();
}
void print(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        print_char(str[i]);
    }
}
void delete_char()
{
    if (cursor_x > 0)
    {
        cursor_x--;
    }
    else if (cursor_y > 0)
    {
        cursor_y--;
        cursor_x = VGA_WIDTH - 1;
    }

    // Clear the character at the current cursor position
    vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (DEFAULT_COLOR << 8) | ' ';
    move_cursor();
}