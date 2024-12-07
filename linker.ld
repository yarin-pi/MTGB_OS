/* linker.ld */
ENTRY(_start)
SECTIONS
{
  . = 0x7e00;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}