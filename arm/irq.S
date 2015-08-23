.arm
.syntax unified

.section .text

.global _vec_irq
_vec_irq:
  /* banked registers for IRQ are sp, lr, and spsr */
  push {r0-r3,r12,lr}
  mrs r0, spsr
  push {r0}

  /* TODO, VFP registers */

  /* GIC base address for this CPU */
  mov  r0, #0x0100
  movt r0, #0x1e00

  /* acknowledge/take responsibility for an active interrupt */
  ldr r2, [r0, #0x0c]
  push {r0,r2} /* need to write this back later for EoI */

  /* valid vectors are [32,96) */
  cmp r2, #95
  bgt _irq_bad
  cmp r2, #32
  blt _irq_bad

  /* table is for vectors 32 through 95 */
  sub r1, r2, #32

  ldr r0, =irq_table

  add r0, r0, r1, LSL #2  /* r0 += r1<<2 */

  ldr r1, [r0]

  mov r0, r2
  blx r1

  /* end active interrupt */
  pop {r0,r2}
  str r2, [r0, #0x10]

_irq_done:
  blx _thread_schedule
  pop {r0}
  msr spsr, r0
  pop {r0-r3,r12,lr}
  subs pc, lr, #4


_irq_bad:
  /* check for spurious interrupt */
  mov r0, #0xff
  orr r0, #0x300
  cmp r2, r0

  movlt r0, r2
  bl invalid_irq_vector
  b _irq_done

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
