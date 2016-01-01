/* link to load from flash device */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH (arm)

ENTRY(_vec_reset)

MEMORY {
  ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 64M
  RAM (rwx) : ORIGIN = 0x60000000, LENGTH = 16M
}

INCLUDE "../common.ld"
