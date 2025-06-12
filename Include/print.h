#ifndef PRINT_H
#define PRINT_H
#include "std.h"
typedef char *va_list;

#define VA_SIZEOF(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define va_start(ap, last) (ap = (va_list) & last + VA_SIZEOF(last))
#define va_arg(ap, type) (*(type *)((ap += VA_SIZEOF(type)) - VA_SIZEOF(type)))
#define va_end(ap) (ap = 0)

void clear_screen();

void move_cursor();
void kprintf(const char *fmt, ...);

void print(const char *str);
void vprint_char(char c);
void vprint(const char *str);
void print_int(uint32_t i, int base);
void delete_char();
void print_float(float num);
void vprint_int(uint32_t i, int base);

#endif PRINT_H // !PRINT_H
