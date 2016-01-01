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
