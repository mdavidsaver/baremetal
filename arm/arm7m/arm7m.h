#ifndef ARM7M_H
#define ARM7M_H

#define M_SYS_BASE 0xe000e000

/* Assume a minimum of 8 available regions
 * the two lowest priority regions are for background ROM and RAM.
 * The highest priority region masks the first 64 bytes to catch *NULL
 */
#define MPU_USER_OFFSET 2
/* up to 4 regions can be updated with a single stmia.
 * user0 - process data/bss
 */
#define MPU_USER_REGIONS 4

#define MPU_XN (1<<28)
#define MPU_AP(N) ((N)<<24)
#define MPU_TEX(N) ((N)<<19)
#define MPU_S (1<<18)
#define MPU_C (1<<17)
#define MPU_B (1<<16)

/* definitions of TEX, S, C, and B
 * based on defaults for TI TM4C1294
 */
#define MPU_STRONG 0
#define MPU_DEVICE (MPU_S|MPU_B)
#define MPU_RAM (MPU_S|MPU_C)
#define MPU_ROM (MPU_C)

#define MPU_AP_NONO MPU_AP(0)
#define MPU_AP_RWNO MPU_AP(1)
#define MPU_AP_RWRO MPU_AP(2)
#define MPU_AP_RWRW MPU_AP(3)
#define MPU_AP_RONO MPU_AP(5)
#define MPU_AP_RORO MPU_AP(6)


#ifndef __ASSEMBLER__

#include "io.h"

static inline
void scs_out32(uint16_t off, uint32_t val)
{
    out32(off + (void*)M_SYS_BASE, val);
}

static inline
uint32_t scs_in32(uint16_t off)
{
    return in32(off + (void*)M_SYS_BASE);
}

#endif

#endif // ARM7M_H
