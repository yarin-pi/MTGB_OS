#include "std.h"

void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


char* int_to_string(uint32_t num, char* str, int base) {
    int i = 0;
    int is_negative = 0;

    // Handle negative numbers for base 10
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    // Process the number
    do {
        int remainder = num % base;
        str[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
        num = num / base;
    } while (num != 0);

    // Add negative sign for negative base 10 numbers
    if (is_negative) {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    // Reverse the string
    reverse(str, i);

    return str;
}
void outl(uint16_t port, uint32_t value) {
    asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 16-bit value from an I/O port
uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Read a 32-bit value from an I/O port
uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void *memset(void *ptr, int value, int num)
{
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}