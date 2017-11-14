#include "asm.h"
#include "spr.h"

/* Relocate all sections as the bootloader has placed us arbitrarily in RAM.
 */

.section .text.start
.global _start
_start:

  // update MSR
  mfmsr %r8
  // disable all exceptions
  load_const %r9, MSR_BOOKE_MASK
  andc %r8, %r8, %r9

  bl startx
startx:

  /* assume r3-r7 hold info from bootloader to be preserved */
  mr %r27, %r3
  mr %r28, %r4
  mr %r29, %r5
  mr %r30, %r6
  mr %r31, %r7

  mflr %r0                /* holds actual address of 'startx' */
  load_const %r1, startx  /* desired location of 'startx' */

  sub. %r26, %r0, %r1 /* actual - desired, aka.  src - dest */

  mfspr %r25, SPR_L1CFG0
  andi. %r25, %r25, L1CFG0_CSIZE /* L1 cache line size (power of 2) */

  /* we are in one of 5 possible arrangements
   * 1. already in right place
   *     |--- dest ---|
   *     |--- src ----|
   * 2. no overlap dest lower
   *     |--- dest ---|
   *                    |--- src ----|
   * 3. no overlap src lower
   *                    |--- dest ---|
   *     |--- src ----|
   * 4. overlap dest lower
   *     |--- dest ---|
   *          |--- src ----|
   * 5. overlap dest lower
   *          |--- dest ---|
   *     |--- src ----|
   *
   * We do a two stage copy.
   * 1. copy codemove() after both src and dest
   * 2. use copied codemove() to copy ourselves
   */

  /* place relocator above max(src, dest)+size */

  load_const %r5, _end /* dest+size */

  cmpwi %r26, 0
  blt usedest
  /* src >= dest */
  add %r5, %r5, %r26 /* src+size */
usedest:

  /* skip to start of next cache line */
  subi %r3, %r25, 1
  or %r5, %r5, %r3
  addi %r5, %r5, 1

  load_const %r3, codemove
  add %r3, %r3, %r26 /* actual codemove */

  load_const %r4, end_codemove - codemove

  mr %r8, %r5 /* save pointer to relocator codemove */
  bl codemove

  /* jump to relocator to copy ourself to final destination.
   * relocator must jump to final destination as this location
   * may be clobbered.
   */

  load_const %r5, __text_start /* dest */
  load_const %r4, _end
  sub %r4, %r4, %r5  /* # of bytes */
  add %r3, %r5, %r26 /* src */

  mtctr %r8 /* branch to relocator (copy of codemove() ) */
  load_const %r8, atfinal /* which returns to atfinal in final location */
  mtlr %r8

  bctr

atfinal:

  mr %r3, %r27
  mr %r4, %r28
  mr %r5, %r29
  mr %r6, %r30
  mr %r7, %r31

  b _pre_Init

/* relocator (memmove() with L1 icache clear)
 *
 * r3 - base address of code to move
 * r4 - size of code to move
 * r5 - address of destination
 *
 * clobbers
 *  ctr
 *  cr0
 *  r3 - r7
 */
codemove:
  cmpwi %r4, 0
  beqlr-

  /* save for later use w/ L1 */
  mr %r6, %r4
  mr %r7, %r5

  /* add one as 'bdnz' equivalent is if(--ctr) */
  addi %r4, %r4, 1
  mtctr %r4
  subi %r4, %r4, 1

  cmpw %r5, %r3
  beq relcd /* dest == src */

  bgt down

up: /* dest < src */
  subi %r3, %r3, 1
  subi %r5, %r5, 1

uploop:
  lbzu %r4, 1(%r3)
  stbu %r4, 1(%r5)
  bdnz+ uploop

  b relcd
down: /* dest > src */
  add %r3, %r3, %r4
  add %r5, %r5, %r4
  addi %r3, %r3, 1
  addi %r5, %r5, 1

downloop:
  lbzu %r4, -1(%r3)
  stbu %r4, -1(%r5)
  bdnz+ downloop

relcd:
  sync
  isync

  /* invalidate destination in L1 instruction cache */
  add %r3, %r7, %r6 /* dest+size */
  mr %r5, %r7       /* dest */

iline:
  dcbst 0, %r5 /* force d-cache write back */
  icbi 0, %r5  /* invalidate i-cache */
  add %r5, %r5, %r25
  cmpw %r5, %r3
  blt+ iline

  sync
  isync

  blr
end_codemove:
