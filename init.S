#include "asm.h"

.section .text
.global _pre_Init
_pre_Init:
  bl _setup_eabi

  bl Init
_stuck:
  b _stuck

.global _setup_eabi
_setup_eabi:
  // update MSR
  mfmsr %r3
  // disable all exceptions
  load_const %r2, MSR_BOOKE_MASK
  andc %r3, %r3, %r2

  // enable floating point
  load_const %r2, MSR_FP
  or %r3, %r3, %r2

  mtmsr %r3

  /* setup registers according to PPC EABI
   * http://www.systemcomputing.org/ppc/ppc3.htm
   */

  /* r1 stack pointer */
  load_const %r1 stack_top

  /* r2 read-only small data area pointer */
  load_const %r2 _SDA2_BASE_

  /* r13 read/write small data area pointer */
  load_const %r13 _SDA_BASE_

  mflr %r31

  bl prepare_data_segments

  mtlr %r31
  blr

.section .bss
stack_bottom:
.skip 2048
.global stack_top
stack_top:
