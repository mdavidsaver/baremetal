/* link to load from ram (bootloader) */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_setup)

MEMORY {
  ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 64M
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16M
}

INCLUDE "../common.ld"
