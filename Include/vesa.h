#ifndef VESA_H
#define VESA_H
#include "std.h"

typedef struct
{
    uint16_t Signature;
    uint32_t FileSize;
    uint32_t Reserved;
    uint32_t DataOffset;

    uint32_t Size;
    uint32_t Width;
    uint32_t Height;
    uint16_t Planes;
    uint16_t BitsPerPixel;
    uint32_t CompressionMethod;
    uint32_t ImageSize;
    uint32_t XPixelsPerM;
    uint32_t YPixelsPerM;
    uint32_t ColorsUsed;
    uint32_t ImportantColors;
} ImageHeader;

// Define the Bitmap struct (similar to the class)
typedef struct
{
    uint8_t *Buffer;
    uint8_t BitsPerPixel;
    uint8_t *Palette;
    uint8_t *Data;
    int bytesPerScanline;

    int Width;
    int Height;
    uint8_t *ImageRGB;
} Bitmap;
struct vbe_info_structure
{
    char signature[4];     // must be "VESA" to indicate valid VBE support
    uint16_t version;      // VBE version; high byte is major version, low byte is minor version
    uint32_t oem;          // segment:offset pointer to OEM
    uint32_t capabilities; // bitfield that describes card capabilities
    uint32_t video_modes;  // segment:offset pointer to list of supported video modes
    uint16_t video_memory; // amount of video memory in 64KB blocks
    uint16_t software_rev; // software revision
    uint32_t vendor;       // segment:offset to card vendor string
    uint32_t product_name; // segment:offset to card model name
    uint32_t product_rev;  // segment:offset pointer to product revision
    uint8_t reserved[222]; // reserved for future expansion
    uint8_t oem_data[256]; // OEM BIOSes store their strings in this area
} __attribute__((packed));

typedef struct
{
    uint16_t attributes;  // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;     // deprecated
    uint8_t window_b;     // deprecated
    uint16_t granularity; // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;        // number of bytes per horizontal line
    uint16_t width;        // width in pixels
    uint16_t height;       // height in pixels
    uint8_t w_char;        // unused...
    uint8_t y_char;        // ...
    uint8_t planes;
    uint8_t bpp;   // bits per pixel in this mode
    uint8_t banks; // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer; // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size; // size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__((packed)) vbe_mode_info_structure;

void put_pixel(int x, int y, uint32_t color);
void init_framebuffer();
void vesa_set_cursor(uint8_t x, uint8_t y);
void draw_background(uint32_t color);
void DrawTransparentChar16(char c, uint32_t x, uint32_t y, uint32_t color);
void DrawText(char *string, uint32_t x, uint32_t y, uint32_t color);
void vesa_newline();
void shift_up();
void vesa_putchar(char c);
void DrawImage(Bitmap *bmp, int locX, int locY, int zoom);

#endif VESA_H
