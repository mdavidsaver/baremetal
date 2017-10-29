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
  mtmsr %r3
  // disable all exceptions
  load_const %r2, MSR_BOOKE_MASK
  andc %r3, %r3, %r2

  // enable floating point
  load_const %r2, MSR_FP
  or %r3, %r3, %r2

  mtmsr %r3

  mflr %r31

  load_const %r1, __bss_start
  load_const %r2, __bss_end
  bl _mzero

  load_const %r1, __sbss_start
  load_const %r2, __sbss_end
  bl _mzero

  load_const %r1, __sbss2_start
  load_const %r2, __sbss2_end
  bl _mzero

  load_const %r1, __data_start
  load_const %r2, __data_end
  load_const %r3, __data_load
  bl _mcopy

  load_const %r1, __sdata_start
  load_const %r2, __sdata_end
  load_const %r3, __sdata_load
  bl _mcopy

  load_const %r1, __sdata2_start
  load_const %r2, __sdata2_end
  load_const %r3, __sdata2_load
  bl _mcopy

  /* setup registers according to PPC EABI
   * http://www.systemcomputing.org/ppc/ppc3.htm
   */

  /* r1 stack pointer */
  load_const %r1 stack_top

  /* r2 read-only small data area pointer */
  load_const %r2 _SDA2_BASE_

  /* r13 read/write small data area pointer */
  load_const %r13 _SDA_BASE_

  mtlr %r31
  blr

_mzero:
  /* r1 start address, r2 end address */
  /* clobbers r4 */
  cmplw %r1, %r2
  bgelr-

  sub %r4, %r2, %r1
  mtctr %r4

  li %r4, 0
  subi %r1, %r1, 1
_mzero_loop:
  stbu %r4, 1(%r1)
  bdnz+ _mzero_loop

  blr

.global _mcopy
_mcopy:
  /* r1 dst start address, r2 dst end address, r3 src start address */
  /* clobbers r4 */
  cmplw %r1, %r2
  bgelr-

  sub %r4, %r2, %r1
  mtctr %r4

  subi %r1, %r1, 1
  subi %r3, %r3, 1

_mcopy_loop:
  lbzu %r4, 1(%r3)
  stbu %r4, 1(%r1)
  bdnz+ _mcopy_loop

  blr

.section .bss
stack_bottom:
.skip 2048
stack_top:
