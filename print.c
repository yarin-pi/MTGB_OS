#include "print.h"
void write_string(int colour)
{
    char* string;
    asm("mov %%ebx, %0" : "=r"(string));
    volatile char *video = (volatile char*)0x3000;
    while( *string != 0 )
    {
        *video++ = *string++;
        *video++ = 0x05;
    }
}