.arm
.syntax unified

.section .text.start
.global _vec_reset

/* CPU exception vector table */
_vec_reset: b _setup
_vec_undef: b _vec_undef
_vec_swi  : b _vec_swi
_vec_prfch: b _vec_prfch
_vec_abort: b _vec_abort
  nop
_vec_irq  : b _vec_irq
_vec_fiq  : b _vec_fiq

.section .text
.global _setup
_setup:
  cpsid if
  /* look for bootloader info (-kernel mode only) */
  cmp r1, #0x8e0 /* boardid for vexpress-a9 */
  /* cmpne r1, #other */
  moveq r4, r1
  movne r4, #0
  moveq r5, r2
  movne r5, #0

  bl _setup_stacks

  /* set exception vector base address (needed when loaded to ram) */
  ldr r0, =_vec_reset
  mcr p15, 0, r0, c12, c0, 0

  mov r0, r4 /* board id */
  mov r1, r5 /* board info */
  blx Init /* jump to C code */

  /* fall through to halt */

.global _halt
_halt:
  /* request shutdown */
  cpsid if

  /* load sysctrl register base */
  mov r0, #0
  movt r0, #0x1000

  /* load command function 8, site 0, device 0 (shutdown) */
  mov r1, #0
  movt r1, #0xc080

  str r1, [r0, #0xa4] /* write SYS_CFGCTRL */

  /* fall through to spin */

_stuck:
  b _stuck

/* output a single char to UART 1 */
.global putchar
putchar:
  ldr r2, =0x10009000

  strb r0, [r2]

  bx lr


_setup_stacks:
  mov r3, lr /* msr clobbers lr */

  cps #18 /* IRQ Mode */
  ldr r0, =_irq_stack_top

  cps #17 /* FIQ Mode */
  ldr sp, =_fiq_stack_top

  cps #31 /* System Mode */
  ldr sp, =_sys_stack_top

  /* do a little test that we can write to the stack.
   * Will fail if the stack was placed in ROM.
   */
  mov r0, #0x42
  push {r0}
  pop {r0}
  cmp r0, #0x42
  bne _halt

  bx r3

.section .bss.stack
.align 4

.skip 0x400
_irq_stack_top:

.skip 0x400
_fiq_stack_top:

.skip 0x400
.global _sys_stack_top
_sys_stack_top:
