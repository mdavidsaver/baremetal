#ifndef ASM_H
#define ASM_H

#ifdef __asm__

#define FUNC(label) .type label, function; label:

#define OBJ(label) .type label, object; label:

#define END(label) .size label, .-label

#define GLOBAL(label) .global label
#define WEAK(label) .weak label

#endif /* __asm__ */

#endif // ASM_H
