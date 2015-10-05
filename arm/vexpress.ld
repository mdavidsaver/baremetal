/* link to load from ram (bootloader) */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_setup)

MEMORY {
  ram (rwx) : ORIGIN = 0x60000000, LENGTH = 16M
}

SECTIONS
{
    /* place holder for bootloader's ram */
    .bootloader (NOLOAD) : {
        __ram_start = .;
        . += 0x10000;
    }

    .text :
    {
      *(.text.start) /* ensure that vector table appears first */
      *(.text .text.*)
    } =0 /* =0 zeros unused */

    .rodata :
    {
      *(.rodata .rodata.*)
    } =0

    /* begin c++ stuff */

    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
    PROVIDE_HIDDEN (__exidx_end = .);

    .preinit_array     :
    {
      PROVIDE_HIDDEN (__preinit_array_start = .);
      KEEP (*(.preinit_array))
      PROVIDE_HIDDEN (__preinit_array_end = .);
    }
    .init_array     :
    {
      PROVIDE_HIDDEN (__init_array_start = .);
      KEEP (*(SORT(.init_array.*)))
      KEEP (*(.init_array ))
      PROVIDE_HIDDEN (__init_array_end = .);
    }
    .fini_array     :
    {
      PROVIDE_HIDDEN (__fini_array_start = .);
      KEEP (*(SORT(.fini_array.*)))
      KEEP (*(.fini_array ))
      PROVIDE_HIDDEN (__fini_array_end = .);
    }
    .jcr            : { KEEP (*(.jcr)) }

    /* end c++ stuff */

    .data ALIGN(4) : ALIGN(4)
    {
      __data_start = .;
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = .;
    }
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4)
    {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.* .gnu.linkonce.b.*)
      __bss_end = .;
    }

    .note.gnu.build-id (NOLOAD) : {
      *(.note.*)
    }

  __after_all_load = ALIGN(0x1000); /* align to a page at start of dynamic memory area */
}
