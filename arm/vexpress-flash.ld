/* link to load from flash device */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_vec_reset)

MEMORY {
  rom (rx)  : ORIGIN = 0x00000000, LENGTH = 64M
  ram (rwx) : ORIGIN = 0x60000000, LENGTH = 16M
}

SECTIONS
{
    .text :
    {
      *(.text.start) /* ensure that vector table appears first */
      . += 0x1000;
      *(.text .text.*)
    } >rom =0 /* =0 zeros unused */

    .rodata :
    {
      *(.rodata .rodata.*)
    } >rom =0

    .data  : ALIGN(4)
    {
      __data_start = ALIGN(4);
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = ALIGN(4);
    } >ram AT>rom
    __data_load = LOADADDR(.data);

  .bss  (NOLOAD) : ALIGN(4)
  {
    __bss_start = ALIGN(4);
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    __bss_end = ALIGN(4);
  } >ram

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
