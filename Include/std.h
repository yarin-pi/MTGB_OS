#ifndef STD_H
#define STD_H
#define HIGHER_HALF(addr) ((void *)((uint32_t)(addr) + 0xC0000000))
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
void reverse(char str[], int length);
char *int_to_string(uint32_t num, char *str, int base);
void outl(uint16_t port, uint32_t value);
extern uint8_t scheduler_enabled;
extern uint32_t time_slice_remaining;

typedef enum
{
    FALSE,
    TRUE
} abool;

typedef enum
{
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED,
    WAITING_FOR_LOCK
} state;

struct interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} __attribute__((packed));
// Read a 16-bit value from an I/O port
uint16_t inw(uint16_t port);

void *memset(void *ptr, int value, int num);
// Read a 32-bit value from an I/O port
uint32_t inl(uint16_t port);
uint32_t strlen(const char *str);
char toupper(char c);
void memcpy(void *dest, const void *src, uint32_t count);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
int strcmp(const char *str1, const char *str2, uint32_t n);
int pow(int a, int b);
char *append_strings(const char *str1, const char *str2);
int fstrcmp(const char *str1, const char *str2);
uint32_t str_to_int(const char *str);

#endif STD_H
