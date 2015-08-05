.arm
.syntax unified

.section .text.start
.global _vec_reset

/* CPU exception vector table */
_vec_reset: b _setup
_vec_undef: b _bad_instr
_vec_swi  : b _vec_swi
_vec_prfch: b _bad_code
_vec_abort: b _bad_data
  nop
_vec_irq  : b _vec_irq
_vec_fiq  : b _vec_fiq

.weak _vec_swi, _vec_irq, _vec_fiq

.section .text
.global _setup
_setup:
  /* look for bootloader info (-kernel mode only) */
  cmp r1, #0x8e0 /* boardid for vexpress-a9 */
  /* cmpne r1, #other */
  ittee eq
  moveq r4, r1
  moveq r5, r2
  movne r4, #0
  movne r5, #0
  /* careful to avoid clobbering r4 and r5 until they can stored */

  bl _cpu_cleanup

  /* show some life */
  ldr r0, =_early_string1
  bl xputs

  /* prepare C environment */
  bl _zero_bss
  bl _move_data
  bl _setup_stacks

  ldr r0, =board_id
  str r4, [r0]

  /* set exception vector base address (needed when loaded to ram) */
  ldr r0, =_vec_reset
  mcr p15, 0, r0, c12, c0, 0

  mov r0, r5 /* board info */
  blx processATAG /* first C function */

  blx irq_setup;

  cpsie i /* enable IRQ */

  ldr r0, =_early_string2
  bl xputs

  blx Init /* jump to "main" */

  /* fall through to halt */

.global halt
.align 4
halt:
  /* request shutdown */

  ldr r0, =_late_string1
  bl xputs

  /* load sysctrl register base */
  ldr r0, =0x10000000

  /* load command function 8, site 0, device 0 (shutdown) */
  ldr r1, =0xc0800000

  str r1, [r0, #0xa4] /* write SYS_CFGCTRL */

  /* fall through to spin */

_stuck:
  b _stuck

/* output a nil terminated string to UART 1 */
xputs:
  sub r0, #1 /* the pointer back by one to use pre-index load below */

  ldr r2, =0x10009000

_puts_loop:
  ldrb r1, [r0, #1]!
  cmp r1, #0
  it ne
  strbne r1, [r2]
  bne _puts_loop

  bx lr

/* Assumes that .bss load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
_zero_bss:
  /* zero bss, aligned to 4 bytes */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end
  sub r2, r1

  sub r1, #4          /* we will use pre-indexed store */

_bss_loop:
  cmp r2, #0
  itt ne
  strne r0, [r1, #4]!  /* *(++r1) = r0 where r1 is uint32_t* */
  subne r2, #4
  bne _bss_loop

  bx lr

/* Assumes that .data load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
_move_data:
  /* move .data section to ram */
  ldr r0, =__data_load
  ldr r1, =__data_start

  /* if load==start then no move is necessary */
  cmp r0, r1
  it eq
  bxeq lr

  ldr r2, =__data_end
  sub r2, r2, r1

  sub r0, #4
  sub r1, #4

_data_loop:
  cmp r2, #0
  itttt ne
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

  orr r1, r0, #27 /* UND Mode */
  msr cpsr, r1
  ldr sp, =_und_stack_top

  orr r1, r0, #23 /* abort Mode */
  msr cpsr, r1
  ldr sp, =_bad_stack_top

  orr r1, r0, #18 /* IRQ Mode */
  msr cpsr, r1
  ldr sp, =_irq_stack_top

  orr r1, r0, #17 /* FIQ Mode */
  msr cpsr, r1
  ldr sp, =_fiq_stack_top

  orr r1, r0, #0x19 /* Supvervisor Mode */
  msr cpsr, r1
  ldr sp, =_svc_stack_top

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
  bne halt

  bx r3

/* "cleanup" means shutting down CPU features which a
 * careless bootloader might leave turned on.
 * QEMU does not do this (so far).
 *
 * based on chapter 3 of the programmers guide.
 */
_cpu_cleanup:
  /* read system control register (SCTLR) */
  mrc p15, 0, r0, c1, c0, 0
  /* Disable MMU and L1 D cache */
  bic r0, #0b0000000000000101
  /* disable L1 I cache and branch prediction */
  bic r0, #0b0001100000000000
  mcr p15, 0, r0, c1, c0, 0

  /* invalidate L1 caches */
  mov r0, #0
  mcr p15, 0, r0, c7, c5, 0 /* read ICIALLU (QEMU nop) */

  /* invalidate data cache */
  /* read CCSIDR cache size id register
   */
  mrc p15, 1, r0, c0, c0, 0
  mov r3, #0x0ff
  orr r3, #0x100
  /* r0 = r3 & (r0>>13) = number of cache sets */
  and r0, r3, r0, lsr #13

  mov r1, #0 /* way counter */
way_loop:

  mov r3, #0 /* set counter */
set_loop:

  mov r2, r1, lsl #30     /* r2  = r1<<30 */
  orr r2, r2, r3, lsl #5  /* r2 |= r2<<5 */
  /* write DCISW to invalidate cache line (QEMU nop) */
  mcr p15, 0, r2, c7, c6, 2
  add r3, #1
  cmp r0, r3
  bne set_loop

  add r1, #1
  cmp r1, #4 /* 4 way L1 cache */
  bne way_loop

  mcr p15, 0, r1, c8, c7, 0 /* write TLBIALL to invalidate TLB */

  bx lr

/* default handler for _vec_undef (undefined instruction) */
.global _bad_instr
_bad_instr:
  sub r3, lr, #4
  ldr r2, [r3]
  ldr r1, =_und_err_msg
  mov r0, #0
  bl printk
  b halt

.weak _bad_instr

.global _bad_code
_bad_code:
  /* TODO, read IFAR, IFSR, and AIDR */
  sub r2, lr, #4
  ldr r1, =_code_err_msg
  mov r0, #0
  bl printk
  b halt

.weak _bad_code

.global _bad_data
_bad_data:
  /* TODO, read DFAR, DFSR, and AIDR */
  sub r2, lr, #8
  ldr r1, =_data_err_msg
  mov r0, #0
  bl printk
  b halt

.weak _bad_data

/* stub for early PIC setup */
.global irq_setup
irq_setup:
  bx lr

.weak irq_setup

/* keep these strings in .text so they are always available */
_early_string1:
.string "booting...\n"
_early_string2:
.string "Enter Init()\n"
_late_string1:
.string "halt\n"

.section .rodata
_und_err_msg:
.string "Invalid instruction 0x%x at 0x%x\n"
_code_err_msg:
.string "Unable to execute instruction at 0x%x\n"
_data_err_msg:
.string "Unable to access memory at 0x%x\n"

.section .bss
.align 4

.skip 0x400
_irq_stack_top:

.skip 0x400
_fiq_stack_top:

.skip 0x400
_und_stack_top:

.skip 0x400
_bad_stack_top:

.skip 0x400
_sys_stack_top:

.skip 0x400
_svc_stack_top:
