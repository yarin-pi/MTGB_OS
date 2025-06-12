ENTRY(_start)

SECTIONS
{
    /* Load kernel code at physical address 0x100000 */
    . = 0xc0100000;
    
    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
        . = ALIGN(0x1000);
    }

    /* Ensure symbol table is placed at the virtual address 0xc0100000 */
   
}
