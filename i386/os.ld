ENTRY(_start)

SECTIONS
{
  ENTRY(_start)

  . = 1024; /* start address of first block */

  /* to be detected, multiboot header must be in the first 8K of the file */
  .text BLOCK(4K) : ALIGN(4K)
  {
    *(.multiboot)
    . = ALIGN(4096); /* give some more space before the actual code starts */
    *(.text)
  }

  .rodata BLOCK(4K) : ALIGN(4K)
  {
    *(.rodata)
  }

  .data BLOCK(4K) : ALIGN(4K)
  {
    *(.data)
    *(.bootstack)
  }

  .bss BLOCK(4K) : ALIGN(4K)
  {
    *(COMMON)
    *(.bss)
    *(.stack)
  }

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
