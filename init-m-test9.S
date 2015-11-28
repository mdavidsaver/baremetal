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

.section .text

.global _start
.thumb_func
_start:
  ldr r3, =early_state

  mov r0, lr
  str r0, [r3], #4

  mrs r0, XPSR
  str r0, [r3], #4

  mrs r0, PRIMASK
  str r0, [r3], #4

  mrs r0, FAULTMASK
  str r0, [r3], #4

  mrs r0, BASEPRI
  str r0, [r3], #4

  mrs r0, CONTROL
  str r0, [r3], #4

  mrs r0, MSP
  str r0, [r3], #4

  mrs r0, PSP
  str r0, [r3], #4

  ldr r2, =0xe000e000

  ldr r0, [r2, #0xd00] /* CPUID */
  str r0, [r3], #4

  ldr r0, [r2, #0xd04] /* ICSR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd08] /* VTOR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd0c] /* AIRCR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd10] /* SCR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd14] /* CCR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd18] /* SHPR1 */
  str r0, [r3], #4
  ldr r0, [r2, #0xd1c] /* SHPR2 */
  str r0, [r3], #4
  ldr r0, [r2, #0xd20] /* SHPR3 */
  str r0, [r3], #4

  ldr r0, [r2, #0xd24] /* SHCSR */
  str r0, [r3], #4

  ldr r0, [r2, #0x10] /* SYST_CSR */
  str r0, [r3], #4

  ldr r0, [r2, #0x4] /* ICTR */
  str r0, [r3], #4

  ldr r0, [r2, #0xd90] /* MPU_TYPE */
  str r0, [r3], #4

  ldr r0, [r2, #0xd94] /* MPU_CTRL */
  str r0, [r3], #4

  ldr r0, =0xdeadbeaf
  str r0, [r3], #4

  blx board_setup
  blx main
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

.thumb_func
_vec_stuck:
  mov r0, #'S'
  ldr r1, =0x4000c000
  str r0, [r1]
  bl abort

.section .rodata
.global hexchars
hexchars:
.string "0123456789abcdef"

.section .bss
.align 4

.global _main_stack_bot
_main_stack_bot:
.skip 0x400
.global _main_stack_top
_main_stack_top:
