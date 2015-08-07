.arm
.syntax unified

.section .text

.global _bad_data
_bad_data:
  /* step the stack point back to make room for 16 gp registers and spsr.
   * store in increasing order
   * we will pass this to the C handler
   */
  sub sp, #17*4
  stmia sp!, {r0-r7}
  /* ok, now some breathing room */

  /* copy our banked registers before mode switch */
  mov r5, sp
  mov r6, lr
  mrs r7, spsr

  /* find mode of the faulting operation */
  and r1, r7, #0x1f

  cmp r1, #23
  it eq
  beq _done /* oh no, its me. break the loop */

  cmp r1, #16
  it eq
  moveq r1, #31 /* substitute system for user mode so we don't lose privlages */

  mrs r2, cpsr
  bic r3, r2, #0x1f
  orr r3, r1

  msr cpsr, r3 /* change mode */

  stmia r5!, {r8-r14}

  msr cpsr, r2 /* restore mode */

  sub r3, r6, #8 /* the address of the faulting instruction */
  stmia r5!, {r3,r7} /* store pc and spsr */

  sub r0, r5, #17*4 /* r0 points to value of r0 at fault */
  mov sp, r0

  mrc p15, 0, r2, c5, c0, 0 /* read DFSR */
  mrc p15, 0, r1, c6, c0, 0 /* read DFAR */

  ldr r4, =exc_data_fault
  blx r4

  mov sp, r5 /* sp back to its entry value */

_done:
  ldr r4, =halt
  bx r4

