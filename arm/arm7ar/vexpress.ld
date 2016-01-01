/* link to load from ram (bootloader) */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_setup)

MEMORY {
  RAM (rwx) : ORIGIN = 0x60000000, LENGTH = 16M
}

REGION_ALIAS("ROM", RAM)

SECTIONS
{
    /* place holder for bootloader's ram */
    .bootloader (NOLOAD) : {
        . += 0x10000;
    } >ROM
}

INCLUDE "../common.ld"

ASSERT(LOADADDR(.bootloader)==ORIGIN(ROM), "LMA doesn't start when it should")
ASSERT(ADDR(.bootloader)==ORIGIN(ROM), "VMA doesn't start when it should")
