#include "print.h"
#include "std.h"
#include "vesa.h"
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xc0150000

// VGA color attributes (example: white text on black background)
#define DEFAULT_COLOR 0x0F

uint16_t *vga_buffer = (uint16_t *)VGA_BUFFER; // VGA buffer pointer
uint8_t cursor_x = 0;                          // Current cursor position (x)
uint8_t cursor_y = 0;
// Current cursor position (y)
void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt)
    {
        if (*fmt == '%' && *(fmt + 1) == 'd')
        {
            int num = va_arg(args, int);
            vprint_int(num, 10);
            fmt++;
        }
        if (*fmt == '%' && *(fmt + 1) == 's')
        {
            const char *str = va_arg(args, char *);
            vprint(str);
            fmt++;
        }
        if (*fmt == '%' && *(fmt + 1) == 'p')
        {
            int ptr = va_arg(args, int);
            vprint_int(ptr, 16);
            fmt++;
        }
        else
        {
            vprint_char(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

void clear_screen()
{
    draw_background(0);
    vesa_set_cursor(0, 0);
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

void vprint_char(char c)
{
    vesa_putchar(c);
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

    for (int i = 0; str[i] != '\0'; i++)
    {
        print_char(str[i]);
    }
}
void vprint(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        vprint_char(str[i]);
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
void vprint_int(uint32_t i, int base)
{
    char *num[32];
    int_to_string(i, num, base);
    vprint(num);
    vprint("\n");
}
void print_int(uint32_t i, int base)
{
    char *no[32];
    int_to_string(i, no, base);
    print(no);
    print("\n");
}
void print_float(float num)
{
    // Handling negative numbers
    if (num < 0)
    {
        print_char('-');
        num = -num; // Make the number positive for further processing
    }

    // Integer part
    uint32_t int_part = (uint32_t)num;
    print_int(int_part, 10); // Print the integer part

    // Fractional part
    num = num - int_part; // Get the fractional part
    if (num > 0)
    {
        print_char('.'); // Add the decimal point
        uint32_t i;
        for (i = 0; i < 2; i++) // Print up to 2 decimal places
        {
            num *= 10;
            uint32_t fractional_digit = (uint32_t)num;
            print_char(fractional_digit + '0'); // Convert the digit to a character
            num -= fractional_digit;
        }
    }
    print("\n"); // New line after printing the float
}