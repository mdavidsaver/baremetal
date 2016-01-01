.arm
.syntax unified

.section .text


/* context switching as a function call, r0 - current, r1 - next */
.global _thread_switch
_thread_switch:
 mrs r3, cpsr
 stmia r0, {r3,r4-r12,sp,lr} /* save cpsr and all general, stack, and link */

_resume:
 ldr r3, [r1], #4
 msr cpsr, r3 /* may change mode */
 ldmia r1, {r4-r12,sp,lr}
 bx lr

/* start running a thread and allow the current context to be lost */
.global _thread_run
_thread_run:
  mov r1, r0
  b _resume

/* new threads "return" here from _thread_run */
.global _thread_start
_thread_start:
  mov r0, r5
  mov r1, #0
  mov r2, #0
  mov r3, #0
  blx r4

  /* user function returns, cleanup */

  cpsid if
  ldr sp, =_thread_cleanup_stack

  blx _thread_cleanup_now /* never returns */

  blx halt

.section .bss

/* a small stack to allow thread cleanup w/ C functions */
.skip 128
_thread_cleanup_stack:
