.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

.section .text.start
.global _start
_start:
  /* capture initial register values */
  load_const %r2, ireg-4
  stwu %r0, 4(%r2)
  stwu %r1, 4(%r2)
  stwu %r1, 4(%r2) /* store r1 again in place of r2 */
  stwu %r3, 4(%r2)
  stwu %r4, 4(%r2)
  stwu %r5, 4(%r2)
  stwu %r6, 4(%r2)
  stwu %r7, 4(%r2)
  stwu %r8, 4(%r2)
  stwu %r9, 4(%r2)
  stwu %r10, 4(%r2)
  stwu %r11, 4(%r2)
  stwu %r12, 4(%r2)
  stwu %r13, 4(%r2)
  stwu %r14, 4(%r2)
  stwu %r15, 4(%r2)
  stwu %r16, 4(%r2)
  stwu %r17, 4(%r2)
  stwu %r18, 4(%r2)
  stwu %r19, 4(%r2)
  stwu %r20, 4(%r2)
  stwu %r21, 4(%r2)
  stwu %r22, 4(%r2)
  stwu %r23, 4(%r2)
  stwu %r24, 4(%r2)
  stwu %r25, 4(%r2)
  stwu %r26, 4(%r2)
  stwu %r27, 4(%r2)
  stwu %r28, 4(%r2)
  stwu %r29, 4(%r2)
  stwu %r30, 4(%r2)
  stwu %r31, 4(%r2)

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

  li %r3,0x2000 /* set power managment bit, clear all others */
  mtmsr %r3

  /* r2 read-only small data area pointer */
  load_const %r2 _SDA2_BASE_

  /* r13 read/write small data area pointer */
  load_const %r13 _SDA_BASE_
  bl Init
_stuck:
  b _stuck

_mzero:
  /* r1 start address, r2 end address */
  cmplw %r1, %r2
  bge- _mzero_done

  sub %r4, %r2, %r1
  mtctr %r4

  li %r4, 0
  subi %r1, %r1, 1
_mzero_loop:
  stbu %r4, 1(%r1)
  bdnz+ _mzero_loop

_mzero_done:
  blr

_mcopy:
  /* r1 dst start address, r2 dst end address, r3 src start address */
  cmplw %r1, %r2
  bge- _mcopy_done

  sub %r4, %r2, %r1
  mtctr %r4

  subi %r1, %r1, 1
  subi %r3, %r3, 1

_mcopy_loop:
  lbzu %r4, 1(%r3)
  stbu %r4, 1(%r1)
  bdnz+ _mcopy_loop

_mcopy_done:
  blr

.section .bss
stack_bottom:
.skip 2048
stack_top:
