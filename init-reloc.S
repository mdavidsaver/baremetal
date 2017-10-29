#include "asm.h"
/* Relocate all sections as the bootloader has placed us arbitrarily in RAM */

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

  sub. %r10, %r0, %r1 /* actual - desired */
  beq done

  load_const %r1, __text_start
  load_const %r2, __text_end
  add %r3, %r1, %r10 /* desired + actual - desired */
  bl _mcopy

  load_const %r1, __rodata_start
  load_const %r2, __rodata_end
  add %r3, %r1, %r10
  bl _mcopy

  load_const %r1, __data_load
  load_const %r2, __data_start
  load_const %r3, __data_end
  sub %r2, %r3, %r2
  add %r2, %r1, %r2
  add %r3, %r1, %r10
  bl _mcopy

  load_const %r1, __sdata_load
  load_const %r2, __sdata_start
  load_const %r3, __sdata_end
  sub %r2, %r3, %r2
  add %r2, %r1, %r2
  add %r3, %r1, %r10
  bl _mcopy

  load_const %r1, __sdata2_load
  load_const %r2, __sdata2_start
  load_const %r3, __sdata2_end
  sub %r2, %r3, %r2
  add %r2, %r1, %r2
  add %r3, %r1, %r10
  bl _mcopy

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
