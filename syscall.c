#include "syscall.h"
#include "print.h"
#include "fs.h"
#include "print.h"
__attribute__((interrupt, target("general-regs-only")))  void handle_syscall(struct interrupt_frame *frame)
{
    switch (get_eax_value())
    {
    case SYS_READ:
        char *path = get_ebx_value();

        getContent(path, get_ecx_value());
        break;
    case SYS_WRITE:
        void *ecx_val = get_ecx_value();
        FatAddFile(get_ebx_value(), ecx_val, strlen((char *)ecx_val));
        break;
    case SYS_PRINT:
        char *text = get_ebx_value();
        print(text);
        break;
    default:
        print("unknown syscall number: ");
        print_int(get_eax_value(), 10);
        break;
    }
}
uint32_t get_eax_value()
{
    int eax_value;
    __asm__ volatile(
        "movl %%eax, %0"
        : "=r"(eax_value) // Output operand: store EAX into eax_value
        :
        : "eax" // Clobber list: tell compiler that EAX is used
    );
    return eax_value;
}
char *get_ebx_value()
{
    char *ebx_value;
    __asm__ volatile(
        "movl %%ebx, %0"
        : "=r"(ebx_value) // Store EBX into ebx_value
        :
        : "ebx" // Tell compiler EBX is used
    );
    return ebx_value;
}
void *get_ecx_value()
{
    char *ecx_value;
    __asm__ volatile(
        "movl %%ecx, %0"
        : "=r"(ecx_value) // Store EcX into ecx_value
        :
        : "ecx" // Tell compiler EcX is used
    );
    return ecx_value;
}