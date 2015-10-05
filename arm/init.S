.arm
.syntax unified

#include "asm.h"
#include "armv7-ar.h"

.section .text.start
.global _vec_reset

/* CPU exception vector table */
_vec_reset: b _setup
_vec_undef: ldr pc,=_bad_instr
_vec_swi  : ldr pc,=_vec_swi
_vec_prfch: ldr pc,=_bad_code
_vec_abort: ldr pc,=_bad_data
  nop
_vec_irq  : ldr pc,=_vec_irq
_vec_fiq  : ldr pc,=_vec_fiq

.weak _vec_swi, _vec_irq, _vec_fiq

.section .text
.global _setup
.type _setup, function
_setup:
.fnstart
.cantunwind
  /* setup system mode stack */
  cpsid if, #ARM_MODE_SYS
  ldr sp, =_sys_stack_top

  /* look for bootloader info (-kernel mode only) */
  cmp r1, #0x8e0 /* boardid for vexpress-a9 */
  /* cmpne r1, #other */
  ittee eq
  moveq r4, r1
  moveq r5, r2
  movne r4, #0
  movne r5, #0
  /* careful to avoid clobbering r4 and r5 until they can be stored */

  /* do a little test that we can write to the stack.
   * Will fail if the stack was placed in ROM.
   */
  mov r0, #0x42
  push {r0}
  pop {r0}
  cmp r0, #0x42
  bne halt

  blx cpu_cleanup /* disable and clear MMU and caches */

  /* show some life */
  adr r0, _early_string1
  bl puts

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
  blx processATAG

  adr r0, _early_string2
  bl puts

  blx __run_init /* run C++ static ctors */

  blx Init /* jump to "main" */

  blx __run_fini /* run C++ static dtors */

  /* fall through to halt */

.global abort
.type abort, function
.align 4

.global halt
.type halt, function
.align 4
abort:
halt:
  ldr sp, =_sys_stack_top

  /* request shutdown */

  ldr r0, =_late_string1
  bl puts

  /* load sysctrl register base */
  ldr r0, =0x10000000

  /* load command function 8, site 0, device 0 (shutdown) */
  ldr r1, =0xc0800000

  str r1, [r0, #0xa4] /* write SYS_CFGCTRL */

  /* fall through to spin */

_stuck:
  b _stuck
.fnend
.size _setup, .-_setup
.size abort, .-abort
.size halt, .-halt

/* Assumes that .bss load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
_zero_bss:
  /* zero bss, aligned to 4 bytes */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end

_bss_loop:
  cmp r1, r2
  itt lt
  strlt r0, [r1], #4  /* *r1++ = r0 where r1 is uint32_t* */
  blt _bss_loop

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

_data_loop:
  cmp r1, r2
  ittt lt
  ldrlt r3, [r0], #4
  strlt r3, [r1], #4
  blt _data_loop

  bx lr

_setup_stacks:
  mov r3, lr /* msr clobbers lr */

  cpsid if, #ARM_MODE_UND
  ldr sp, =_und_stack_top

  cpsid if, #ARM_MODE_ABT
  ldr sp, =_und_stack_top

  cpsid if, #ARM_MODE_IRQ
  ldr sp, =_irq_stack_top

  cpsid if, #ARM_MODE_FIQ
  ldr sp, =_fiq_stack_top

  cpsid if, #ARM_MODE_SVC
  ldr sp, =_svc_stack_top

  cpsid if, #ARM_MODE_SYS

  bx r3

/* default handler for _vec_undef (undefined instruction) */
.global _bad_instr
_bad_instr:
  push {r0-r3,r12,lr}
  mrs r0, spsr
  push {r0}

  tst r0, #(1<<5) /* test thumb */
  it eq
  subeq r3, lr, #4 /* ARM */
  subne r3, lr, #2

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
  sub r2, lr, #8
  ldr r1, =_data_err_msg
  mov r0, #0
  bl printk
  b halt

.weak _bad_data

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
