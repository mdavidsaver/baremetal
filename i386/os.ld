ENTRY(_start)

SECTIONS
{
  ENTRY(_start)

  . = 1M;

  /* to be detected, multiboot header must be in the first 8K of the file */
  .text BLOCK(4K) : ALIGN(4K)
  {
    PROVIDE(__text_start = .);
    *(.multiboot)
    . = ALIGN(4096); /* give some more space before the actual code starts */
    *(.text)
    PROVIDE(__text_end = .);
  }

  .rodata BLOCK(4K) : ALIGN(4K)
  {
    PROVIDE(__rodata_start = .);
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    PROVIDE(__rodata_end = .);
  }

  .data BLOCK(4K) : ALIGN(4K)
  {
    PROVIDE(__data_start = .);
    *(.data .data.* .gnu.linkonce.d.*)
    PROVIDE(__data_end = .);
  }

  .bss BLOCK(4K) : ALIGN(4K)
  {
    PROVIDE(__bss_start = .);
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.bss.stack)
    PROVIDE(__bss_end = .);
  }
  PROVIDE(__heap_start = .);

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
