.arm
.syntax unified

#include "armv7-ar.h"

.section .text

/* Call on entry to an exception handler
 * push the registers of the interrupted state
 * onto this exception mode stack
 * Move the stack pointer back by 17*4 bytes
 */
.macro except_entry mymode=ARM_MODE_ABT
  /* make room for 16 registers + spsr */
  sub sp, #17*4
  stmia sp!, {r0-r7} /* store the first registers which are never banked */

  /* copy our banked registers before mode switch */
  mov r5, sp
  mov r6, lr
  mrs r7, spsr

  /* find mode of the faulting operation */
  and r1, r7, #ARM_MODE_MASK

  cmp r1, #\mymode
  it eq
  ldreq r1, =halt
  bxeq r1 /* oh no, its me. break the loop */

  cmp r1, #ARM_MODE_USR
  it eq
  moveq r1, #ARM_MODE_SYS /* substitute system for user mode so we don't lose privlages */

  mrs r2, cpsr
  bic r3, r2, #0x1f
  orr r3, r1

  msr cpsr, r3 /* change mode */

  stmia r5!, {r8-r14}

  msr cpsr, r2 /* restore mode */

  stmia r5!, {r6,r7} /* store pc and spsr */
  sub sp, r5, #17*4 /* points back to the top of the stack (value of r0) */
.endm

.global _bad_data
_bad_data:
  except_entry ARM_MODE_ABT

  sub r3, lr, #8 /* the address of the faulting instruction */

  mrc p15, 0, r2, c5, c0, 0 /* read DFSR */
  mrc p15, 0, r1, c6, c0, 0 /* read DFAR */

  ldr r4, =exc_data_fault
  blx r4

  add sp, #17*5 /* sp back to its entry value */

  ldr r4, =halt
  bx r4

.global _bad_code
_bad_code:
  except_entry ARM_MODE_ABT

  sub r3, lr, #4 /* the address of the faulting instruction */

  mrc p15, 0, r2, c5, c0, 1 /* read IFSR */
  mrc p15, 0, r1, c6, c0, 2 /* read IFAR */

  ldr r4, =exc_prefetch_fault
  blx r4

  add sp, #17*5 /* sp back to its entry value */

  ldr r4, =halt
  bx r4
