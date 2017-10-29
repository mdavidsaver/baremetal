#ifndef ASM_H
#define ASM_H

.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

#define MSR_WE (1<<(63-45))
#define MSR_CE (1<<(63-46))
#define MSR_EE (1<<(63-48))
#define MSR_PR (1<<(63-49))
#define MSR_FP (1<<(63-50))
#define MSR_ME (1<<(63-51))
#define MSR_FE0 (1<<(63-52))
#define MSR_DE (1<<(63-54))
#define MSR_FE1 (1<<(63-55))
#define MSR_IS (1<<(63-58))
#define MSR_DS (1<<(63-59))

#define MSR_BOOKE_MASK (MSR_WE|MSR_CE|MSR_EE|MSR_PR|MSR_FP|MSR_ME|MSR_FE0|MSR_DE|MSR_FE1|MSR_IS|MSR_DS)

#endif /* ASM_H */
