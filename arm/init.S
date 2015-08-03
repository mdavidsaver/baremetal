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
  /* look for bootloader info (-kernel mode only) */
  cmp r1, #0x8e0 /* boardid for vexpress-a9 */
  /* cmpne r1, #other */
  moveq r4, r1
  movne r4, #0
  moveq r5, r2
  movne r5, #0

  /* show some life */
  ldr r0, =_early_string1
  bl xputs

  /* prepare C environment */
  bl _zero_bss
  bl _move_data
  bl _setup_stacks

  /* set exception vector base address (needed when loaded to ram) */
  ldr r0, =_vec_reset
  mcr p15, 0, r0, c12, c0, 0

  ldr r0, =_early_string2
  bl xputs

  mov r0, r4 /* board id */
  mov r1, r5 /* board info */
  blx Init /* jump to C code */

  /* fall through to halt */

.global _halt
.align 4
_halt:
  /* request shutdown */

  ldr r0, =_late_string1
  bl xputs

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
.global xputchar
.align 4
xputchar:
  mov r2, #0x9000
  movt r2, #0x1000

  strb r0, [r2]

  bx lr

/* output a nil terminated string to UART 1 */
.global xputs
.align 4
xputs:
  sub r0, #1 /* the pointer back by one to use pre-index load below */

  mov r2, #0x9000
  movt r2, #0x1000

_puts_loop:
  ldrb r1, [r0, #1]!
  cmp r1, #0
  strbne r1, [r2]
  bne _puts_loop

  bx lr

.global xnputs
.align 4
xnputs:
  /* r0 is char*, r1 is size */
  sub r0, #1

  mov r3, #0x9000
  movt r3, #0x1000

_xnputs_loop:
  cmp r1, #0
  ldrbne r2, [r0, #1]!
  strbne r2, [r3]
  subne r1, #1
  bne _xnputs_loop

  bx lr

_zero_bss:
  /* zero bss, aligned to 4 bytes */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end
  sub r2, r2, r1

  sub r1, #4          /* we will use pre-indexed store */

_bss_loop:
  cmp r2, #0
  strne r0, [r1, #4]!
  subne r2, #4
  bne _bss_loop

  bx lr

_move_data:
  /* move .data section to ram */
  ldr r0, =__data_load
  ldr r1, =__data_start

  /* if load==start then no move is necessary */
  cmp r0, r1
  bxeq lr

  ldr r2, =__data_end
  sub r2, r2, r1

  sub r0, #4
  sub r1, #4

_data_loop:
  cmp r2, #0
  ldrne r3, [r0, #4]!
  strne r3, [r1, #4]!
  subne r2, #4
  bne _data_loop
  bx lr

_setup_stacks:
  mov r3, lr /* msr clobbers lr */

  /* setup CPSR */
  mrs r0, cpsr
  bic r0, r0, #0x1f /* clear mode bits */
  orr r0, r0, #0xc0 /* set to disable IRQ and FIQ */

  orr r1, r0, #18 /* IRQ Mode */
  msr cpsr, r1
  ldr sp, =_irq_stack_top

  orr r1, r0, #17 /* IRQ Mode */
  msr cpsr, r1
  ldr sp, =_fiq_stack_top

  orr r1, r0, #0x1f /* System Mode */
  msr cpsr, r1
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

/* keep these strings in .text so they are available
 * before .data is moved
 */
_early_string1:
.string "booting...\r\n"
_early_string2:
.string "ready for C...\r\n"
_late_string1:
.string "halt\r\n"

.section .bss
.align 4

.skip 0x400
_irq_stack_top:

.skip 0x400
_fiq_stack_top:

.skip 0x400
.global _sys_stack_top
_sys_stack_top:
