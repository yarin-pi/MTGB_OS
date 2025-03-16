#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#include "std.h"
#include "print.h"
uint32_t *framebuffer;
uint16_t pitch;
uint8_t bpp;
void put_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        // framebuffer[y * SCREEN_WIDTH + x] = color;
        uint32_t pixel_offset = y * pitch + (x * (bpp / 8));                           // Compute offset
        uint32_t *pixel_address = (uint32_t *)((uint8_t *)framebuffer + pixel_offset); // Get the correct address
        *pixel_address = color;
    }
}
void init_framebuffer()
{
    framebuffer = (uint32_t *)(*(uint32_t *)0x9000); // Read framebuffer address correctly
    bpp = *(uint8_t *)0x9012;
    pitch = *(uint16_t *)0x9010;
}
void draw_test_pattern()
{
    // Start with a color value, and decrease it for each pixel
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            // Draw the pixel with the current color
            put_pixel(x, y, 0x003300FF);
        }
    }
}