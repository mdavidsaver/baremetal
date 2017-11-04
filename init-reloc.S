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

  mflr %r0 /* holds actual address of 'startx' */
  load_const %r1, startx  /* desired location of 'startx' */

  sub. %r26, %r0, %r1 /* actual - desired */
  beq done

  load_const %r1, stack_top
  load_const %r2, _SDA2_BASE_
  load_const %r13, _SDA_BASE_

  load_const %r3, __text_start /* desired */
  load_const %r5, __text_end
  add %r4, %r3, %r26 /* actual = desired + actual - desired */
  sub %r5, %r5, %r3  /* count */
  bl memmove

  load_const %r3, __rodata_start /* desired */
  load_const %r5, __rodata_end
  add %r4, %r3, %r26 /* actual = desired + actual - desired */
  sub %r5, %r5, %r3  /* count */
  bl memmove

  load_const %r3, __data_load /* desired */
  load_const %r4, __data_start
  load_const %r5, __data_end
  sub %r5, %r5, %r4 /* count */
  add %r4, %r3, %r26 /* actual = desired + actual - desired */
  bl memmove

  load_const %r3, __sdata_load /* desired */
  load_const %r4, __sdata_start
  load_const %r5, __sdata_end
  sub %r5, %r5, %r4 /* count */
  add %r4, %r3, %r26 /* actual = desired + actual - desired */
  bl memmove

  load_const %r3, __sdata2_load /* desired */
  load_const %r4, __sdata2_start
  load_const %r5, __sdata2_end
  sub %r5, %r5, %r4 /* count */
  add %r4, %r3, %r26 /* actual = desired + actual - desired */
  bl memmove

  sync
  isync

  load_const %r1, _pre_Init
  mtlr %r1

  mr %r3, %r27
  mr %r4, %r28
  mr %r5, %r29
  mr %r6, %r30
  mr %r7, %r31

done:
  blr
