
SECTIONS
{
    __ram_start = ORIGIN(RAM);

    .text :
    {
      *(.text.start) /* ensure that vector table appears first */
      *(.text .text.*)
    } >ROM =0 /* =0 zeros unused */

    .rodata :
    {
      *(.rodata .rodata.*)
    } >ROM =0

    /* begin c++ stuff */

    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ROM
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >ROM
    PROVIDE_HIDDEN (__exidx_end = .);

    .preinit_array     :
    {
      PROVIDE_HIDDEN (__preinit_array_start = .);
      KEEP (*(.preinit_array))
      PROVIDE_HIDDEN (__preinit_array_end = .);
    } >ROM
    .init_array     :
    {
      PROVIDE_HIDDEN (__init_array_start = .);
      KEEP (*(SORT(.init_array.*)))
      KEEP (*(.init_array ))
      PROVIDE_HIDDEN (__init_array_end = .);
    } >ROM
    .fini_array     :
    {
      PROVIDE_HIDDEN (__fini_array_start = .);
      KEEP (*(SORT(.fini_array.*)))
      KEEP (*(.fini_array ))
      PROVIDE_HIDDEN (__fini_array_end = .);
    } >ROM
    .jcr            : { KEEP (*(.jcr)) } >ROM

    /* end c++ stuff */
    
    .data : ALIGN(4)
    {
      __data_start = .;
      __data_load = LOADADDR(.data);
      *(.data .data.* .gnu.linkonce.d.*)
      __data_end = .;
    } >RAM AT>ROM

    .bss : ALIGN(4)
    {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.* .gnu.linkonce.b.*)
      __bss_end = .;
    } >RAM AT>ROM

    
    __after_all_load = ALIGN(0x1000); /* align to a page at start of dynamic memory area */
}


/* check some assumptions */
ASSERT(ADDR(.text)+SIZEOF(.text)<=ADDR(.rodata), "overlap between .text and .rodata")
ASSERT(ADDR(.rodata)+SIZEOF(.rodata)<=ADDR(.preinit_array), "overlap between .rodata and .preinit_array")
ASSERT(ADDR(.data)+SIZEOF(.data)<=ADDR(.bss), "overlap between .data and .bss")
ASSERT(ADDR(.bss)+SIZEOF(.bss)<=__after_all_load, "overlap between .bss and __after_all_load")

/* _bss_zero() and _move_data() assume word alignment */

ASSERT(__data_start%4==0, "__data_start alignment")
ASSERT(__data_load%4==0, "__data_load alignment")
ASSERT(__data_end%4==0, "__data_end alignment")

ASSERT(__bss_start%4==0, "__bss_start alignment")
ASSERT(__bss_end%4==0, "__bss_end alignment")
