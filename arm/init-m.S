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
  blx Init
  
  /* fall through to abort() */

.global abort
.thumb_func
abort:
  ldr sp, =_main_stack_top

  ldr r0, =0x05fa0004 /* reset request */
  ldr r1, =0xe000ed0c /* AIRCR */
  str r0, [r1]
  dsb

_stuck:
  b _stuck

/* output a single char to UART 1 */
.global putchar
.thumb_func
putchar:
  ldr r2, =0x4000c000

  strb r0, [r2]

  bx lr

.thumb_func
_vec_stuck:
  mov r0, #'S'
  ldr r1, =0x4000c000
  str r0, [r1]
  bl abort

.section .bss.stack
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
