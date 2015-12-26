OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
              "elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

MEMORY {
  ram (rwx) : ORIGIN = 0x0, LENGTH = 64K
  rom (rx) : ORIGIN = 0xff800000, LENGTH = 8M
}

SECTIONS
{
  .rodata :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  } >rom

  .sdata :
  {
    PROVIDE(__sdata_start = .);
    PROVIDE(_SDA_BASE_ = .);
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    PROVIDE(__sdata_end = .);
  } >ram AT>rom
  __sdata_load = LOADADDR(.sdata);

  .sbss :
  {
    PROVIDE(__sbss_start = .);
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    PROVIDE(__sbss_end = .);
  } >ram AT>rom

  .sdata2 :
  {
    PROVIDE(__sdata2_start = .);
    PROVIDE(_SDA2_BASE_ = .);
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    PROVIDE(__sdata2_end = .);
  } >ram AT>rom
  PROVIDE(__sdata2_load = LOADADDR(.sdata2));

  .sbss2 :
  {
    PROVIDE(__sbss2_start = .);
    *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
    PROVIDE(__sbss2_end = .);
  } >ram AT>rom

  .data :
  {
    PROVIDE(__data_start = .);
    *(.data .data.* .gnu.linkonce.d.*)
    PROVIDE(__data_end = .);
  } >ram AT>rom
  PROVIDE(__data_load = LOADADDR(.data));

  .bss :
  {
    PROVIDE(__bss_start = .);
    *(COMMON)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(.stack)
    PROVIDE(__bss_end = .);
  } >ram AT>rom

  .text :
  {
	  __text_start = .;
      *(.text)

      /* as per the mpc8540 ref. manual, assume that after reset the TLB
       * only maps the last 4kb and place early setup code here.
       */
      . += 0xfffff000 - ABSOLUTE(.);
      *(.text.early)

      /* reset vector */
      . += 0xfffffffc - ABSOLUTE(.);
      *(.text.init)
  } >rom
}
