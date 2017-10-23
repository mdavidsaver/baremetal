OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
              "elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_jump_start)

/* QEMU places the first byte of the -bios image file
 * at 0xfff00000 and execution begins at 0xfff00100 (PPC upper reset vector)
 * The length is actually the file length, but is set to 16MB here
 *
 * The -kernel image will be loaded to 0x1000000.  So we just jump there
 */
MEMORY {
  ram (rwx) : ORIGIN = 0x1000000, LENGTH = 1M
  rom (rx) : ORIGIN = 0xfff00000, LENGTH = 16M
}

SECTIONS
{
  .text :
  {
      . = 0x100; /* skip to the reset ISR vector */
      *(.text.start) /* ensure that _jump_start() appears first */
  } >rom

  .text.os (NOLOAD) :
  {
      *(.text.os)
  } >ram

  .note.gnu.build-id (NOLOAD) : {
    *(.note.*)
  }
}
