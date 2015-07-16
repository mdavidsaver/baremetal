OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
              "elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_jump_start)

/* QEMU places the first byte of the -bios image file
 * at 0xfff00000 and execution begins at 0xfff00100.
 * The length is actually the file length, but is set to 16MB here
 *
 * Code sections are executed directly from the file (>rom).
 * Data sections are loaded from rom, but executed in ram (>ram AT>rom).
 */
MEMORY {
  /* 'ram' can be marked non-executable (omit 'x') since .text will remain in 'rom' */
  ram (rw) : ORIGIN = 0, LENGTH = 128M
  rom (rx) : ORIGIN = 0xfff00000, LENGTH = 16M
}

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

  .text :
  {
    . = 0x100; /* skip to the reset ISR vector */
    *(.text.start) /* ensure that _start() appears first */
    . = 0x20000; /* skip past ISR vectors */
    *(.text)
  } >rom =0 /* =0 zeros unused rom areas */

  .rodata :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    /* store load (LMA) addresses for >ram sections */
  } >rom =0

  .vectors (NOLOAD) :
  {
    . = . + 0x2000; /* skip */
  } >ram

  .sdata :
  {
    __sdata_start = .;
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    __sdata_end = .;
  } >ram AT>rom
  __sdata_load = LOADADDR(.sdata);

  .sbss (NOLOAD) :
  {
    __sbss_start = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    __sbss_end = .;
  } >ram AT>rom

  .sdata2 :
  {
    __sdata2_start = .;
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    __sdata2_end = .;
  } >ram AT>rom
  __sdata2_load = LOADADDR(.sdata2);

  .sbss2 (NOLOAD) :
  {
    __sbss2_start = .;
    *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
    __sbss2_end = .;
  } >ram AT>rom

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
