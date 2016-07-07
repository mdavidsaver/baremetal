.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

.section .text
.global _start
_start:
  /* setup registers according to PPC EABI
   * http://www.systemcomputing.org/ppc/ppc3.htm
   */

  /* r1 stack pointer */
  load_const %r1, stack_top

  li %r3,0x2000 /* set power managment bit, clear all others */
  mtmsr %r3

  bl __eabi

  bl Init
_stuck:
  b _stuck

.global __eabi
__eabi:
  /* load small data area pointers */
  load_const %r13, _SDA_BASE_
  load_const %r2 , _SDA2_BASE_
  blr

.section .bss
stack_bottom:
.skip 16384
stack_top:
