/* link to load from ram (bootloader) */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_setup)

MEMORY {
  rom (rx)  : ORIGIN = 0x00000000, LENGTH = 64M
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 16M
}

SECTIONS
{
    .text :
    {
      *(.text.start) /* ensure that vector table appears first */
      . = ALIGN(0x1000); /* skip to the next page */
      *(.text .text.*)
    } >rom =0 /* =0 zeros unused */

    .rodata :
    {
      *(.rodata .rodata.*)
    } >rom =0

    /* begin c++ stuff */

    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >rom
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >rom
    PROVIDE_HIDDEN (__exidx_end = .);

    .preinit_array     :
    {
      PROVIDE_HIDDEN (__preinit_array_start = .);
      KEEP (*(.preinit_array))
      PROVIDE_HIDDEN (__preinit_array_end = .);
    } >rom
    .init_array     :
    {
      PROVIDE_HIDDEN (__init_array_start = .);
      KEEP (*(SORT(.init_array.*)))
      KEEP (*(.init_array ))
      PROVIDE_HIDDEN (__init_array_end = .);
    } >rom
    .fini_array     :
    {
      PROVIDE_HIDDEN (__fini_array_start = .);
      KEEP (*(SORT(.fini_array.*)))
      KEEP (*(.fini_array ))
      PROVIDE_HIDDEN (__fini_array_end = .);
    } >rom
    .jcr            : { KEEP (*(.jcr)) } >rom

    /* end c++ stuff */

    .data  : ALIGN(4)
    {
      __data_start = .;
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = .;
    } >ram AT>rom
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4)
    {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.* .gnu.linkonce.b.*)
      __bss_end = .;
    } >ram

    .note.gnu.build-id (NOLOAD) : {
      *(.note.*)
    }

  __after_all_load = ALIGN(0x1000); /* align to a page at start of dynamic memory area */
}
