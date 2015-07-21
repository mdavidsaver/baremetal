.section .text
.global _start
_start:
  /* setup registers according to PPC EABI
   * http://www.systemcomputing.org/ppc/ppc3.htm
   */

  /* r1 stack pointer */
  lis %r1,stack_top@h
  ori %r1,%r1,stack_top@l

  li %r3,0x2000 /* set power managment bit, clear all others */
  mtmsr %r3

  /* r2 read-only small data area pointer */
  lis %r2,_SDA2_BASE_@h
  ori %r2,%r2,_SDA2_BASE_@l

  /* r13 read/write small data area pointer */
  lis %r2,_SDA_BASE_@h
  ori %r2,%r2,_SDA_BASE_@l
  bl Init
_stuck:
  b _stuck

.section .bss
stack_bottom:
.skip 16384
stack_top:
