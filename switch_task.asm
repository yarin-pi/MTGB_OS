section .text
extern current_task_TCB
extern tss_entry
extern postpone_task_switches_counter
extern task_switches_postponed_flag
global switch_to_task
switch_to_task:

    cmp dword [postpone_task_switches_counter],0
    je .continue
    mov dword [task_switches_postponed_flag],1
    ret
.continue:
    push ebx
    push esi
    push edi
    push ebp

    mov edi,[current_task_TCB]    ;edi = address of the previous task's "thread control block"
    mov [edi + 8],esp         ;Save ESP for previous task's kernel stack in the thread's TCB

    ;Load next task's state

    mov esi,[esp+(4+1)*4]         ;esi = address of the next task's "thread control block" (parameter passed on stack)
    mov [current_task_TCB],esi    ;Current task's TCB is the next task TCB
        
    mov eax,[esi]         ;eax = address of page directory for next task
    mov ebx,[esi+12]        ;ebx = address for the top of the next task's kernel stack
    mov [tss_entry + 4],ebx            ;Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
    mov ecx,cr3                   ;ecx = previous task's virtual address space

    cmp eax,ecx                   ;Does the virtual address space need to being changed?
    je .doneVAS                   ; no, virtual address space is the same, so don't reload it and cause TLB flushes
    mov cr3,eax                   ; yes, load the next task's virtual address space
.doneVAS:

    pop ebp
    pop edi
    pop esi
    pop ebx

    ret  