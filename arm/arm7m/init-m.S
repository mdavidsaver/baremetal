#include "asm.h"
#include "arm7m.h"

.thumb
.syntax unified

/* boot time vector table */
.section .text.start
.global _boot_reset
.align 7 /* 128 bytes */
object _boot_reset
.word _main_stack_top
.word _setup  /* Reset */
.word _stuck_vec  /* NMI */
.word hardfault_handler  /* HardFault */
.word memfault_handler  /* MemManage */
.word busfault_handler  /* BusFault */
.word usagefault_handler /* UsageFault */
.word _stuck_vec  /* Reserved */
.word _stuck_vec  /* Reserved */
.word _stuck_vec  /* Reserved */
.word _stuck_vec  /* Reserved */
.word svc_handler  /* SVCall */
.word _stuck_vec  /* Debug Mon. */
.word _stuck_vec  /* Reserved */
.word _stuck_vec  /* PendSV */
.word systick_handler  /* SysTick */
endobject _boot_reset

.section .text

/* when debugging read 0xe000ed04 to get vector number */
funct _stuck_vec
  b _stuck_vec
endfunct _stuck_vec

.global _setup
funct _setup
.fnstart
.cantunwind
  /* mask all faults until uart is setup */
  cpsid if
  mov r0, #0
  msr basepri, r0

  /* prepare C environment */
  bl _zero_bss
  bl _move_data

  ldr r0, =_proc_stack_top
  msr psp, r0

  mov r0, #2 /* switch to process stack, retain privlages */
  msr control, r0
  isb

  /* C part of initialization. this function should not return */
  blx setup_c

  /* fall through to halt if it does */

.global halt
funct halt
  cpsid if /* ignored if unpriv */

  mrs r0, PRIMASK  /* unprivlaged always reads as zero */
  cmp r0, #0
  itt eq
  svceq #0  /* SVC thread exit */
  beq _stuck /* should never be reached */

  /* priv continues */

  /* request shutdown */

  ldr r1,=0xe000e000 /* SCS base */
  ldr r0, =0x05fa0004 /* external reset */
  str r0, [r1, 0xd0c] /* AIRCR */
  dsb

  /* fall through to spin */

_stuck:
  b _stuck
.fnend
endfunct halt
endfunct _setup


/* Assumes that .bss load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
funct _zero_bss
  /* zero bss, aligned to 4 bytes */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end

_bss_loop:
  cmp r1, r2
  itt lt
  strblt r0, [r1], #1
  blt _bss_loop

  bx lr
endfunct _zero_bss

funct _move_data
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
  ldrblt r3, [r0], #1
  strblt r3, [r1], #1
  blt _data_loop

  bx lr
endfunct _move_data

.section .bss
.align 2

.skip 0x400
_main_stack_top:

.skip 0x400
_proc_stack_top:
