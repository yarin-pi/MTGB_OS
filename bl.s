section .text
[BITS 16]
[org 0x7C00]



global main
.bpb:
    
    db 0xeb,0x28,0x90
    dq 0x9090909090909090
    dw 512
    db 1
    dw 0x80
    db 1
    dw 512
    dw 0
    db 0xf8
    dw 250
    dw 63
    dw 255
    dd 0
    dd 3000000
    db 0x80
    db 0
    db 0x29
    dd 0xa0a1a2a3
    
main:
sti
vbe_get_info:
        mov ax, 0x4F00           ; VBE function to get information
        lea di, [vbe_info]       ; Load the address of vbe_info into DI
        int 0x10                 ; Call BIOS interrupt to get VBE information

        cmp ax, 0x4F             ; Check if call was successful
        jne error 
 

get_mode_info:
        mov ax, 0x4F01           ; VBE function to get mode information
        mov di, 0x8000           ; Set DI to point to 0x9000 for mode_info storage

        mov cx, 0x4118   
        int 0x10                 ; Call BIOS interrupt to get mode information and store it at 0x9000

        cmp ax, 0x4F             ; Check if the call was successful
        jne error 

set_mode:
        mov ax, 0x4F02           ; VBE function to set video mode
        mov bx, 0x4118
        mov cx, bx    
        or bx, 0x4000            ; Enable linear framebuffer
        int 0x10                 ; Call BIOS interrupt to set the video mode

        cmp ax, 0x4F             ; Check if the call was successful
        jne error  

 

        mov dl, 0x80
        mov dh, 0
        mov ch, 0
        mov cl, 2
        mov ah, 0x02
        mov al, 0x50
        xor bx, bx
        mov di, 0xffff
        mov es, di
        mov bx, 0x0010
        int 0x13
        jmp check_a20
after_a20_valid:
        mov al, [save_above]
        mov [es:di], al
        mov al, [save_below]
        mov [ds:si], al
        sti 
        pop ds
        pop si
        pop di
        pop es
        popf
        cli
        lgdt [gdt_pointer]
        mov eax, cr0
        or al, 1
        mov cr0, eax
        jmp 0x08:protected_mode

error:
        mov ah, 0x00;                   // restart instruction
        int 19h;                        // restart the syste
        ret
check_a20:
        pushf  
        push es
        push di
        push si
        push ds
        cli
        mov ax, 0
        mov ds, ax
        mov si, 0x0500
        not ax
        mov es, ax
        mov di, 0x0510
        mov al, [ds:si]
        mov byte [save_below], al
        mov al, [es:di]
        mov byte [save_above], al
        mov ah, 1
        mov byte [ds:si], 0xfe
        mov byte [es:di], 0xf0
        mov al, [ds:si]
        cmp al, [es:di]
        jne after_a20_valid
        in al, 0x92
        or al, 2
        out 0x92, al
        jmp after_a20_valid
save_above:
    db 0
save_below:
    db 0

[BITS 32]    
protected_mode:
        cli
        mov eax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov esp, 0xffff
        mov ebp, esp
        mov eax, 0x08
        jmp 0x08:0x100000

gdt_start:
   
    ; Null Descriptor
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    ; Code Segment Descriptor
    dw 0xFFFF         ; Limit (64KB)
    dw 0x0000         ; Base (part 1)
    db 0x00           
    db 0x9A           ; Access Byte (Code Segment, Present, Ring 0, Execute/Read)
    db 0xCF           ; Granularity (4KB granularity)
    db 0x00           
    ; Data Segment Descriptor
    dw 0xFFFF         ; Limit (64KB)
    dw 0x0000         ; Base (part 1)
    db 0x00           
    db 0x92           ; Access Byte (Data Segment, Present, Ring 0, Read/Write)
    db 0xCF           ; Granularity (4KB granularity)
    db 0x00                  

gdt_end:
gdt_pointer:
    dw gdt_end - gdt_start - 1  
    dd gdt_start    
    dw 0 
          
.partition_table:
    ; First partition entry (16 bytes)
    times 446 - ($-$$) db 0
    db 0x80                ; Bootable partition
    db 0x00, 0x02, 0x00    ; CHS address (Cylinder/Head/Sector)
    db 0x4d              ; Partition type (Linux)
    db 0xFF, 0xFF, 0xFF    ; CHS address of last sector
    dw 1                   ; Start sector (LBA) for your kernel (sector 1)
    dd 0xFFFFFFFF          ; Partition size (max size for the rest of the disk)

    ; Empty partition entries (for simplicity, we'll leave them empty)
    times 48 db 0 
.done:
    times 510 - ($-$$) db 0
    dw 0xAA55  
section .bss
    vbe_info resb 64     
    mode_info resb 128 