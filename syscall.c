#include "syscall.h"
#include "print.h"
#include "fs.h"
#include "print.h"
#include "exe.h"
#include "scheduler.h"
int tmE = 110;
void wait(int n, int f)
{
    asm volatile("sti");
    while (n > (time_since_boot + f * 100))
        ;
    return;
}
__attribute__((interrupt, target("general-regs-only"))) void handle_syscall(struct interrupt_frame *frame)
{
    uint32_t edi = get_edi_value();
    uint32_t esi = get_esi_value();
    uint32_t ecx = get_ecx_value();
    if (scheduler_enabled)
    {
        if (frame->cs & 0x3 && frame->sp < 0xf0000000)
        {
            // kprintf("i want to kill myself \n");
            current_task_TCB->eip = frame->ip;
            current_task_TCB->stack = frame->sp;
            // kprintf("esp: %p \n",frame->sp );
        }
        lock_stuff();
    }
    set_kernel_stack(k_stack);
    switch (edi)
    {
    case SYS_READ:
        char *path = (char *)esi;
        getContent(path, ecx);
        break;
    case SYS_WRITE:
        void *ecx_val = ecx;
        FatAddFile(esi, ecx_val, strlen((char *)ecx_val));
        break;
    case SYS_PRINT:

    {
        char *text = esi;
        vprint(text);
    }
    break;
    case SYS_PRINT_INT:
        vprint_int(ecx, 10);
        asm volatile("movl %0, %%ecx" ::"r"(0) : "%ecx");
    case SYS_TIMER:
        int sec = esi;
        int i = 0;
        if (scheduler_enabled)
        {
            current_task_TCB->sleep_expiry = time_since_boot + sec * 100;
            unlock_stuff();
            block_task(BLOCKED);
        }
        else
        {

            // outb(0x20,0x20);
            // wait(tmE,sec);
        }
        break;
    default:
        kprintf("unknown syscall number: %d", edi);
        break;
    }
    if (scheduler_enabled)
    {
        unlock_stuff();
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

__attribute__((interrupt, target("general-regs-only"))) void switch_to_kernel(struct interrupt_frame *frame)
{
    while (1)
        ;
    // lock_scheduler();
    outb(0x20, 0x20);
    // unlock_stuff();
    // terminate_task();
    // unlock_scheduler();
}
