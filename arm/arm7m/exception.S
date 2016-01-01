#include "asm.h"
#include "process.h"

.thumb
.syntax unified

.global svc_handler
funct svc_handler
  /* read the instruction which triggered us to get the SVC#
   * we use privlaged reads under the assumption that
   * the memory involved has been "validated" by the cpu
   * in that we are in the SVC handler and not bus/memfault
   */
  push {lr}
  mrs r0, PSP /* preserve r0 to pass to SVC function */

  blx svc_handler_c
  pop {lr}

  b thread_switch

endfunct svc_handler

.global systick_handler
funct systick_handler
  push {lr}
  blx systick_handler_c
  pop {lr}
  b thread_switch
endfunct systick_handler

funct thread_switch
  /* no need to recompute if returning to another handler */
  and r1, lr, #0xf
  cmp r1, #1
  beq switch_done

  /* check if thread actually changes */
  ldr r0, =thread_scheduler
  ldr r1, [r0] /* current thread */
  ldr r2, [r0, #4] /* next thread */
  cmp r1, r2
  beq switch_done

  /* save remaining registers for current thread */
  mrs r0, PSP

  stmdb r0, {r4-r11} /* TODO: may fault or clobber if stack overflows */

  blx thread_do_switch
  mov lr, r0

  mrs r0, PSP

  /* restore non-automatic registers for next thread */
  sub r0, r0,#4*8
  ldmia r0!, {r4-r11}

switch_done:
  bx lr
endfunct thread_switch
