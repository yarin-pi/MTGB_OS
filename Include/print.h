#ifndef PRINT_H
#define PRINT_H
#include "std.h"
void clear_screen();

void move_cursor();

void print(const char *str);
void print_int(uint32_t i,int base);
void delete_char();
#endif PRINT_H // !PRINT_H
