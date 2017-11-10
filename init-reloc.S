#include "asm.h"
/* Relocate all sections as the bootloader has placed us arbitrarily in RAM.
 *
 * TOOD: doesn't handle overlap
 */

.section .text.start
.global _start
_start:
  /* assume r3-r7 hold info from bootloader to be preserved */

  bl startx
startx:

  mr %r27, %r3
  mr %r28, %r4
  mr %r29, %r5
  mr %r30, %r6
  mr %r31, %r7

  mflr %r0                /* holds actual address of 'startx' */
  load_const %r1, startx  /* desired location of 'startx' */

  sub. %r26, %r0, %r1 /* actual - desired, aka.  src - dest */

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
   */

  /* place relocator above max(src, dest)+size */

  load_const %r5, _end /* dest+size */

  cmpwi %r26, 0
  blt usedest
  /* src >= dest */
  add %r5, %r5, %r26 /* src+size */
usedest:

  /* ensure alignment */
  ori %r5, %r5, 0x3
  addi %r5, %r5, 1

  load_const %r3, mmove
  add %r3, %r3, %r26 /* actual mmove */

  load_const %r4, end_mmove - mmove

  mr %r8, %r5 /* save pointer to relocator mmove */
  bl mmove

  /* jump to relocator to copy ourself to final destination.
   * relocator must jump to final destination as this location
   * may be clobbered.
   */

  load_const %r5, __text_start /* dset */
  load_const %r4, _end
  sub %r4, %r4, %r5
  add %r3, %r5, %r26 /* src */

  mtctr %r8 /* branch to relocator (copy of mmove() ) */
  load_const %r8, atfinal /* return to atfinal in final location */
  mtlr %r8

  bctr

atfinal:

  mr %r3, %r27
  mr %r4, %r28
  mr %r5, %r29
  mr %r6, %r30
  mr %r7, %r31

  b _pre_Init

/* relocator
 * r3 - base address of code to move
 * r4 - size of code to move
 * r5 - address of destination
 *
 * clobbers
 *  ctr
 *  cr0
 *  r3 - r5
 */
mmove:
  cmpwi %r4, 0
  beqlr-

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
  /* TODO: L1 cache invalidate */
  sync
  isync
  blr
end_mmove:
