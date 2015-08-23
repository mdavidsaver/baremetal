.arm
.syntax unified

.section .text

.global _vec_irq
_vec_irq:
  /* banked registers for IRQ are sp, lr, and spsr */
  push {r0-r3,r12,lr}
  mrs r0, spsr
  push {r0}

  /* TODO, VFP registers? */

  blx isr_dispatch

  blx _thread_schedule

  pop {r0}
  msr spsr, r0
  pop {r0-r3,r12,lr}
  subs pc, lr, #4

/* thread scheduler stub */
.global _thread_schedule
_thread_schedule:
  bx lr

.weak _thread_schedule

.global irq_mask
irq_mask:
  mrs r1, cpsr
  orr r0, r1, #0x80
  msr cpsr, r0
  and r0, r1, #0x80
  /* returns the previous value of the irq mask bit */
  /* return 0 if interrupt just disabled, 1 if previously disabled */
  dmb
  bx lr

.global irq_unmask
irq_unmask:
  dmb
  cmp r0, #0
  bxne lr
  mrs r1, cpsr
  bic r1, #0x80
  msr cpsr, r1
  bx lr
