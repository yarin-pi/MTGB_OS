#include "std.h"
#include "vm.h"
uint32_t time_slice_remaining = 0;
uint8_t scheduler_enabled = 0;

void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char *int_to_string(uint32_t num, char *str, int base)
{
    int i = 0;
    int is_negative = 0;

    // Handle negative numbers for base 10
    if (num < 0 && base == 10)
    {
        is_negative = 1;
        num = -num;
    }

    // Process the number
    do
    {
        int remainder = num % base;
        str[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
        num = num / base;
    } while (num != 0);

    // Add negative sign for negative base 10 numbers
    if (is_negative)
    {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    // Reverse the string
    reverse(str, i);
    return str;
}
void outl(uint16_t port, uint32_t value)
{
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read a 16-bit value from an I/O port
uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Read a 32-bit value from an I/O port
uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void *memset(void *ptr, int value, int num)
{
    unsigned char *p = ptr;
    while (num--)
    {
        *p++ = (unsigned char)value;
    }
    return ptr;
}
uint32_t str_to_int(const char *str)
{
    int res = 0, sign = 1, i = 0;
    if (str[0] == '-')
        sign = -1, i++;
    for (; str[i] >= '0' && str[i] <= '9'; i++)
        res = res * 10 + (str[i] - '0');
    return res * sign;
}
uint32_t strlen(const char *str)
{
    uint32_t len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    return c;
}
void memcpy(void *dest, const void *src, uint32_t count)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    uint32_t i;
    for (i = 0; i < count; ++i)
    {
        d[i] = s[i];
    }
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
int strcmp(const char *str1, const char *str2, uint32_t n)
{
    uint32_t i = 0;

    while (i < n && str1[i] != '\0' && str2[i] != '\0')
    {
        if (str1[i] != str2[i])
            return (str1[i] - str2[i]);
        i++;
    }

    if (i < n)
    {
        if (str1[i] == '\0' && str2[i] != '\0')
            return -1; // str1 is shorter
        if (str2[i] == '\0' && str1[i] != '\0')
            return 1; // str2 is shorter
    }

    return 0;
}
int pow(int a, int b)
{
    int x = 1;
    for (int i = 0; i < b; i++)
    {
        x = a * x;
    }
    return x;
}

uint32_t get_string_size(char *str)
{
    uint32_t size = 0;

    // Loop through the string until we encounter the null-terminator
    while (str[size] != '\0')
    {
        size++;
    }

    return size;
}

char *append_strings(const char *str1, const char *str2)
{
    // Get the sizes of the two strings
    uint32_t len1 = 0, len2 = 0;
    while (str1[len1] != '\0')
        len1++;
    while (str2[len2] != '\0')
        len2++;

    // Allocate memory for the combined string (+1 for null-terminator)
    char *result = (char *)kalloc(len1 + len2 + 1);
    if (!result)
    {
        return 0; // Handle memory allocation failure
    }

    // Copy the first string
    for (uint32_t i = 0; i < len1; i++)
    {
        result[i] = str1[i];
    }

    // Append the second string
    for (uint32_t i = 0; i < len2; i++)
    {
        result[len1 + i] = str2[i];
    }

    // Null-terminate the result
    result[len1 + len2] = '\0';

    return result;
}
int fstrcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2, 8);
}