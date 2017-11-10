#ifndef EXC_H
#define EXC_H

#define EXC_CRIT 0
#define EXC_MC 1
#define EXC_DS 2
#define EXC_IS 3
#define EXC_EXT 4
#define EXC_ALIGN 5
#define EXC_PROG 6
#define EXC_NOFPU 7
#define EXC_SYSCALL 8
#define EXC_NOAPU 9
#define EXC_DEC 10
#define EXC_TIMER 11
#define EXC_WD 12
#define EXC_DTLB 13
#define EXC_ITLB 14
#define EXC_DEBUG 15

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct exc_frame {
    uint32_t gpr[32],
             cr,
             ctr,
             lr;
} exc_frame;

#endif

#endif // EXC_H
