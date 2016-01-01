
#include "common.h"

/* "cleanup" means shutting down CPU features which a
 * careless bootloader might leave turned on.
 * QEMU does not do this (so far).
 *
 * based on chapter 3 of the cortex-a9 programmers guide.
 */
void cpu_cleanup(void)
{
    unsigned W, S, nways, nsets;
    /* Disable MMU, L1 caches (I and D), and branch prediction */
    {
        uint32_t ctrl;
        asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(ctrl):: "memory"); /* read SCTLR */
        ctrl &= ~0b0001100000000101;
        asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(ctrl): "memory"); /* write SCTLR */
    }

    /* invalidate L1 caches */
    {
        uint32_t ctrl = 0; /* ignored */
        asm volatile("mcr p15, 0, %0, c7, c5, 0" :: "r"(ctrl): "memory"); /* write ICIALLU */
    }

    {
        uint32_t val;
        asm volatile("mrc p15, 1, %0, c0, c0, 0" : "=r"(val):: "memory"); /* read CCSIDR */
        nways = BMASK(12-3)&(val>>3);
        nsets = BMASK(27-13)&(val>>13);
        /* QEMU gives these as zeros */
    }

    /* loop through and invalidate all cache entries */
    for(W=0; W<nways; W++) {
        for(S=0; S<nsets; S++) {
            uint32_t cmd = (W<<30)|(S<<5); /* level 1 */
            asm volatile("mcr p15, 0, %0, c7, c6, 2" :: "r"(cmd): "memory"); /* write DCISW */
        }
    }

    /* invalidate TLB */
    {
        uint32_t ctrl = 0; /* ignored */
        asm volatile("mcr p15, 0, %0, c8, c7, 0" :: "r"(ctrl): "memory"); /* write TLBIALL */
    }
}
