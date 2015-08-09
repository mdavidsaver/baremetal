/* A small bootloader which copies the main program to RAM
 * and sets up ATAG information
 */
#ifndef RAM_SIZE
#  error Must -DRAM_SIZE to match qemu "-m <ramsize>"
#endif

#ifdef BOARD_VEXPRESSA9
#  define BOARD_ID 0x8e0
#else
#  error Must -DBOARD_* to match qemu "-M <machinename>"
#endif

#ifndef BIN_FILE
#  error must -DBIN_FILE
#endif

.arm
.syntax unified

.section .text.start

.global _vec_reset
_vec_reset: b _setup
_vec_undef: b _vec_undef
_vec_swi: b _vec_swi
_vec_prfch: b _vec_prfch
_vec_abort: b _vec_abort
 nop
_vec_irq: b _vec_irq
_vec_fiq: b _vec_fiq

_setup:
  ldr sp, =_boot_stack

  ldr r4, =0x60000000 /* RAM Base address */

  /* copy in board info */
  mov r0, r4
  ldr r1, =_atag_start
  ldr r2, =_atag_end
  sub r2, r1
  bl _memcpy

  add r4, #0x10000

  /* copy in program */
  mov r0, r4
  ldr r1, =_program_start
  ldr r2, =_program_end
  sub r2, r1
  bl _memcpy

  sub r2, r4, #0x10000
  ldr r1, =BOARD_ID
  mov r0, 0

  bx r4

/* r0 dest, r1 src, r2 count */
_memcpy:
  cmp r0, r1
  it eq
  bxeq lr

  sub r0, #1
  sub r1, #1

_memcpy_loop:
  cmp r2, #0
  it eq
  bxeq lr
  ldrb r3, [r1, #1]!
  strb r3, [r0, #1]!
  sub r2, #1
  b _memcpy_loop


/* cf. hw/arm/boot.c in QEMU source */
_atag_start:
.word 5
.word 0x54410001 /* ATAG_CORE */
.word 1
.word 0x1000
.word 0


.word 4
.word 0x54410002 /* ATAG_MEM */
.word 1024*1024*RAM_SIZE
.word 0x60000000

.word 0
.word 0
_atag_end:


.section .bss
.skip 0x1000
_boot_stack:

.section .rodata
_program_start:
.incbin BIN_FILE
_program_end:
