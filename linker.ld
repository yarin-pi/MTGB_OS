ENTRY(_start)

SECTIONS
{
    /* Virtual kernel start address (higher half) */
    . = 0x100000;

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
        /* Align to page size */
        . = ALIGN(0x1000);
    }
}
