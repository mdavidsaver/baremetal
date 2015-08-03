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

    .data :
    {
    . += ALIGN(4);
      __data_start = .;
      *(.data .data.* .gnu.linkonce.d.*)
      . += ALIGN(4);
      __data_end = .;
    } >ram AT>rom
    __data_load = LOADADDR(.data);

  .bss (NOLOAD) :
  {
    . += ALIGN(4);
    __bss_start = .;
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    . += ALIGN(4);
    __bss_end = .;
  } >ram

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
