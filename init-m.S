.thumb
.syntax unified

.section .text.start
.global _boot_reset
.align 7
_boot_reset:
.word _main_stack_top
.word _start /* reset */
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck

.section .data
.global run_table
.align 7
run_table:
.word _main_stack_top
.word _vec_stuck /* reset */
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck


.section .text

.global _start
.thumb_func
_start:
  /* zero bss */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end

_bss_loop:
  cmp r1, r2
  itt ne
  strbne r0, [r1], 1
  bne _bss_loop

  /* copy data */
  ldr r1, =__data_start
  ldr r2, =__data_end
  ldr r3, =__data_load

_data_loop:
  cmp r1, r2
  ittt ne
  ldrbne r0, [r3], 1
  strbne r0, [r1], 1
  bne _data_loop

  ldr r0, =run_table
  ldr r1, =0xe000e000
  str r0, [r1, #0xd08] /* VTOR */

  mov r0, _proc_stack_top
  msr PSP, r0

  blx board_setup
  blx main
  
  /* fall through to abort() */

.global abort
.thumb_func
abort:
  cpsid if

  ldr sp, =_main_stack_top

  ldr r0, =0x05fa0004 /* reset request */
  ldr r1, =0xe000e000
  str r0, [r1, #0xd0c] /* AIRCR */
  dsb

_stuck:
  b _stuck


.thumb_func
_vec_stuck:
  mov r0, #'S'
  ldr r1, =0x4000c000
  str r0, [r1]
  mrs r0, IPSR
  str r0, [r1]
  bl abort

.section .bss
.align 4

.global _main_stack_bot
_main_stack_bot:
.skip 0x400
.global _main_stack_top
_main_stack_top:

.global _proc_stack_bot
_proc_stack_bot:
.skip 0x400
.global _proc_stack_top
_proc_stack_top:
