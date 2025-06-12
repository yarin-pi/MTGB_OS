
#include "std.h"
#include "vesa.h"
#include "print.h"
#include "vm.h"
#include "font.h"
uint32_t framebuffer;
uint16_t pitch;
uint8_t bpp;
uint32_t width;
uint32_t height;
uint32_t currx;
uint32_t curry;
uint32_t current_fg, current_bg;
uint8_t *FontBuffer;
/* the linear framebuffer */

/* number of bytes in each line, it's possible it's not screen width * bytesperpixel! */
/* import our font that's in the object file we've created above */

#define CHAR_ROW 16
#define CHAR_COLUMN 8
#define CHARLEN (CHAR_ROW * CHAR_COLUMN) // 8 columns * 16 rows
#define CHARCOUNT 94                     // 94 printable characters

uint32_t chars[CHARCOUNT * CHARLEN];

void put_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        // framebuffer[y * SCREEN_WIDTH + x] = color;
        uint32_t *pixel_offset = y * pitch + (x * (bpp / 8)) + framebuffer;
        *pixel_offset = color;
    }
}
void put_pixelrgb(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        // Combine r, g, b into a single uint32_t value (assuming 32-bit color with 8 bits per channel)
        uint32_t color = (r << 16) | (g << 8) | b;

        // Calculate the pixel offset in the framebuffer
        uint32_t *pixel_offset = (y * pitch) + (x * (bpp / 8)) + framebuffer;

        // Set the pixel color
        *pixel_offset = color;
    }
}
void init_framebuffer()
{
    vbe_mode_info_structure *mode = (vbe_mode_info_structure *)0x8000;

    if (mode->attributes & 0x80)
    { // Check if linear framebuffer is supported
        framebuffer = mode->framebuffer;
        pitch = mode->pitch;
        bpp = mode->bpp;
        width = mode->width;
        height = mode->height;
        map_page(framebuffer, 0x800000, 1, page_table);
        framebuffer = 0x800000;
        FontBuffer = font16x8;
        vesa_set_cursor(0, 0);
    }
    else
    {
        print("Linear framebuffer not supported or address is zero");
    }
}
void shift_up()
{
    memcpy(((char *)framebuffer) + ((1280 * 16) * 4), ((char *)framebuffer) + ((1280 * 16) * 8), (1280 * 704) * 4 - ((1280 * 16) * 4));
    memset(((char *)framebuffer) + (1280 * 704 * 4), 0, 1280 * 16 * 4);
}

void vesa_newline()
{
    currx = 0;
    if (curry >= 704)
    {
        curry = 704;
        shift_up();
    }
    else
    {
        curry += 16;
    }
}
void vesa_set_cursor(uint8_t x, uint8_t y)
{
    currx = x * 8;
    curry = y * 16;
}

void vesa_putchar(char c)
{
    unsigned char uc = (unsigned char)c;
    if (currx + CHAR_COLUMN >= width)
    { // If the character won't fit on the current line
        vesa_newline();
    }

    switch (c)
    {
    case '\n': // New line
        vesa_newline();
        break;
    case '\t': // Tab
        vesa_putchar(' ');
        vesa_putchar(' ');
        vesa_putchar(' ');
        vesa_putchar(' ');
        break;
    case 0x08: // Backspace
        if (currx > 0)
        {
            if (currx > 0)
            {
                currx -= CHAR_COLUMN; // Move cursor back
                // Overwrite character by filling its area with the background color
                for (int cy = 0; cy < CHAR_ROW; cy++)
                {
                    for (int cx = 0; cx < CHAR_COLUMN; cx++)
                    {
                        put_pixel(currx + cx, curry + cy, current_bg);
                    }
                }
            }
        }
        break;
    default: // Regular character
        // drawchar_transparent(uc, currx, curry, 0xFFFFFFFF); // Draw the character
        DrawTransparentChar16(c, currx, curry, 0xFFFFFFFF);
        currx += CHAR_COLUMN; // Move the cursor position by 8 pixels (width of the character)
        break;
    }
}
void draw_background(uint32_t color)
{

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Draw the pixel with the current color
            put_pixel(x, y, color);
        }
    }
}
void DrawTransparentChar16(char c, uint32_t x, uint32_t y, uint32_t color)
{
    int cx, cy;
    int mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};
    unsigned char *gylph = FontBuffer + (int)c * 16;

    for (cy = 0; cy < 16; cy++)
    {
        for (cx = 0; cx < 8; cx++)
        {
            if (gylph[cy] & mask[cx])
                put_pixel(x + cx, y + cy, color);
        }
    }
}
void DrawText(char *string, uint32_t x, uint32_t y, uint32_t color)
{
    for (int i = 0; string[i] != 0; i++)
    {
        DrawTransparentChar16(string[i], x + (i * 8), y, color);
    }
}
// Function to draw a string of text
void DrawImage(Bitmap *bmp, int locX, int locY, int zoom)
{
    int xpos = 0, ypos = 0, x, y;
    int zoomX, zoomY;

    for (y = 0; y < bmp->Height; y++)
    {
        for (int x = 0; x < bmp->Width * 3; x += 3)
        {
            for (zoomX = 0; zoomX < zoom; zoomX++)
            {
                for (zoomY = 0; zoomY < zoom; zoomY++)
                {
                    put_pixelrgb(locX + xpos, locY + ypos, bmp->ImageRGB[(y * bmp->Width * 3 + x)],
                                 bmp->ImageRGB[(y * bmp->Width * 3 + x + 1)],
                                 bmp->ImageRGB[(y * bmp->Width * 3 + x + 2)]);
                    ypos++;
                }
                xpos++;
                ypos -= zoom;
            }
        }
        xpos = 0;
        ypos += zoom;
    }
}