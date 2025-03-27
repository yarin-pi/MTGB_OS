#include "syscall.h"
#include "print.h"
#include "fs.h"
#include "print.h"
#include "exe.h"

__attribute__((interrupt, target("general-regs-only"))) void handle_syscall(struct interrupt_frame *frame)
{
    uint32_t edi = get_edi_value();
    uint32_t esi = get_esi_value();
    uint32_t ecx = get_ecx_value();
    set_kernel_stack(k_stack);
    switch (edi)
    {
    case SYS_READ:
        char *path = (char*)esi;
        getContent(path, ecx);
        break;
    case SYS_WRITE:
        void *ecx_val = ecx;
        FatAddFile(esi, ecx_val, strlen((char *)ecx_val));
        break;
    case SYS_PRINT:
        char *text = esi;
        print(text);
        break;
    default:
        kprintf("unknown syscall number: %d", edi);
        break;
    }
}
uint32_t get_edi_value()
{
    uint32_t edi_value;
    __asm__ volatile(
        "movl %%edi, %0"
        : "=r"(edi_value) // Output operand: store EAX into eax_value
        :
        : "edi" // Clobber list: tell compiler that EAX is used
    );
    return edi_value;
}
char *get_esi_value()
{
    char *esi_value;
    __asm__ volatile(
        "movl %%esi, %0"
        : "=r"(esi_value) // Store EBX into ebx_value
        :
        : "esi" // Tell compiler EBX is used
    );
    return esi_value;
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
void do_null()
{
    while(1);
}
__attribute__((interrupt, target("general-regs-only"))) void switch_to_kernel(struct interrupt_frame* frame)
{
    do_null();
}

