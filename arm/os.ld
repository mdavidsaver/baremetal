OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_start)

MEMORY {
  ram (rwx) : ORIGIN = 0, LENGTH = 128M
  /* QEMU loads the -kernel file at offset 0x10000 */
  rom (rwx) : ORIGIN = 0x10000, LENGTH = 128M
}

SECTIONS
{
    .junk (NOLOAD) :
    {
        . = . + 0x2000; /* skip */
    }

    .text :
    {
      *(.text.start) /* ensure that _start() appears first */
      *(.text)
    } >rom =0 /* =0 zeros unused rom areas */

    .data :
    {
      __data_start = .;
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = .;
    } >ram AT>rom
    __data_load = LOADADDR(.data);

  .bss (NOLOAD) :
  {
    __bss_start = .;
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.stack)
    __bss_end = .;
  } >ram

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
