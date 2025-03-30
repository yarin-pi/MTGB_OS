#ifndef SYSCALL_H
#define SYSCALL_H
#define SYS_READ 1
#define SYS_WRITE 2
#define SYS_PRINT 3
#define SYS_PRINT_INT 4
#define SYS_TIMER 7

#include "std.h"
void handle_syscall(struct interrupt_frame *frame);
char *get_esi_value();
uint32_t get_edi_value();
void *get_ecx_value();
void switch_to_kernel(struct interrupt_frame *frame);

#endif