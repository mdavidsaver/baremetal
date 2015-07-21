OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
              "elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

SECTIONS
{
  /* Sections
   *
   *ROM
   * .text - regular program text
   *   .text.start - our own creation which holds _start()
   * .init
   * .fini
   * .got2
   * .ctors
   * .dtors
   * .jcr
   * .rodata - read-only data
   *
   *RAM
   * .vectors
   * .sdata2 - r/w small data area (_SDA2_BASE_)
   * .sbss2
   * .data
   * .sdata - r/w small data area (_SDA_BASE_)
   * .sbss
   * .bss
   *   .stack - our program stack
   * .gcc_except_table
  */

  /* placeholder for ISR vectors */
  . = 0x2000;

  /* TODO, do something with the ~16 MB between the vectors and .text */

  . = 0x1000000;

  .text :
  {
    *(.text.start) /* ensure that _start() appears first */
    *(.text)
  } =0 /* =0 zeros unused rom areas */

  .rodata :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    /* store load (LMA) addresses for sections */
  } =0

  .sdata :
  {
    __sdata_start = .;
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    __sdata_end = .;
  } =0
  __sdata_load = LOADADDR(.sdata);

  .sbss (NOLOAD) :
  {
    __sbss_start = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    __sbss_end = .;
  } =0

  .sdata2 :
  {
    __sdata2_start = .;
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    __sdata2_end = .;
  } =0
  __sdata2_load = LOADADDR(.sdata2);

  .sbss2 (NOLOAD) :
  {
    __sbss2_start = .;
    *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
    __sbss2_end = .;
  } =0

  .data :
  {
    __data_start = .;
    *(.data .data.* .gnu.linkonce.d.*)
    __data_end = .;
  } =0
  __data_load = LOADADDR(.data);

  .bss (NOLOAD) :
  {
    __bss_start = .;
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.stack)
    __bss_end = .;
  }

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
