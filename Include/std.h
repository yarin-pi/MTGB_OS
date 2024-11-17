#ifndef STD_H
#define STD_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
void reverse(char str[], int length);
char* int_to_string(uint32_t num, char* str, int base);
void outl(uint16_t port, uint32_t value);


// Read a 16-bit value from an I/O port
uint16_t inw(uint16_t port) ;

void *memset(void *ptr, int value, int num);
// Read a 32-bit value from an I/O port
uint32_t inl(uint16_t port) ;

#endif STD_H
