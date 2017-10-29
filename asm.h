#ifndef ASM_H
#define ASM_H

.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

#endif /* ASM_H */
