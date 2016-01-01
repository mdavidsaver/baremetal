#ifndef ASM_H
#define ASM_H

#ifdef __ASSEMBLER__

.macro funct n
.type \n STT_FUNC
.thumb_func
\n:
.endm

.macro endfunct n
.size \n, .-\n
.endm

.macro object n
.type \n STT_OBJECT
\n:
.endm

.macro endobject n
.size \n, .-\n
.endm

#define END(LBL) .size LBL, .-LBL

#define GLOBAL(label) .global label
#define WEAK(label) .weak label

#endif /* __asm__ */

#endif // ASM_H
