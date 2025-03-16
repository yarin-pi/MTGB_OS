[BITS 16]
[org 0x7C00]

global main

; BIOS Parameter Block (BPB) for disk structure, typically for FAT32 booting
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
    ; Step 1: Check for VESA support
    mov ax, 0x4F00                   ; Get VESA information
    mov di, vesa_info_block
    int 0x10
    cmp ax, 0x4F                     ; Check if VESA is supported
    jne no_vesa

    ; Step 2: Get mode information
    mov ax, 0x4F01                   ; Get mode info
    mov cx, 0x4118                   ; Mode 1024x768x32bpp
    mov di, vesa_mode_info
    int 0x10
    cmp ax, 0x4F                     ; Check if successful
    jne no_vesa

    mov ax, 0x4F02                   ; Set VESA mode
    mov bx, 0x4118                   ; 1024x768x32bpp + LFB
    or bx, 0x4000                    ; Enable linear framebuffer
    int 0x10
    cmp ax, 0x4F                    
    jne no_vesa

    mov ax, [vesa_mode_info + 40]   ; Lower 16 bits
    mov dx, [vesa_mode_info + 42]   ; Upper 16 bits
    shl edx, 16
    or  eax, edx

    jmp check_a20
    mov [0x9000], eax
    mov ax, [vesa_mode_info + 16]  ; Pitch (Bytes per row)
    mov [0x9010], ax

    mov al, [vesa_mode_info + 26]  ; BPP (Bits per pixel)
    mov [0x9012], al
no_vesa:
    ; If VESA mode or protected mode setup fails, halt
    jmp $

after_a20_valid:
    ; After A20 line is valid, we can proceed with protected mode
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


check_a20:
    ; A20 line checking and enabling
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


save_above: db 0
save_below: db 0

[BITS 32]
protected_mode:
    ; Prepare for switching to protected mode
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

vesa_info_block:
    times 32 db 0
vesa_mode_info:
    times 48 db 0

; Global Descriptor Table (GDT) setup
gdt_start:
    ; Null Descriptor
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A            ; Code Segment Descriptor
    db 0xCF
    db 0x00
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92            ; Data Segment Descriptor
    db 0xCF
    db 0x00

gdt_end:

gdt_pointer:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    dw 0

; Padding for boot sector size
times 510 - ($-$$) db 0
dw 0xAA55               ; Boot signature
