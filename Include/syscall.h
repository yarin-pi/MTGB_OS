#ifndef SYSCALL_H
#define SYSCALL_H
#define SYS_READ 1
#define SYS_WRITE 2
#define SYS_PRINT 3

#include "std.h"
void handle_syscall(struct interrupt_frame *frame);
char *get_ebx_value();
uint32_t get_eax_value();
void *get_ecx_value();

#endif