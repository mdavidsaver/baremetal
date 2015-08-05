
#include "mmu.h"

#define BIGPAGE (1<<20)

#define ROM0_BASE 0
#define ROM0_SIZE 0x04000000

#define IO1_BASE 0x10000000
#define IO1_SIZE 0x000e5000

#define IO2_BASE 0x1e000000
#define IO2_SIZE 0x0000b000

#define RAM_BASE 0x60000000

static uint32_t mmu_L1_table[4096] __attribute__((aligned(4*4096)));

/* Create a 1-to-1 mapping to prevent access to undefined addresses */
static
void mmu_build_L1(void)
{
    uint32_t i, ramsize = RamSize;

    if(!ramsize) {
        ramsize = 64*(1<<20);
        printk(0, "Unknown RAM size, assuming %d\n", (int)ramsize);
    }

    /* Create read-only 1MB entries for ROM 0 alias */
    for(i=ROM0_BASE; i<ROM0_BASE+ROM0_SIZE; i+=BIGPAGE)
    {
        uint32_t ent = (i&0xfff00000) | 2;
        ent |= (1<<16) | (1<<3) | (1<<2); /* sharable, cachable, and bufferable (Normal memory) */
        ent |= (1<<15) | (2<<10); /* read-only both privlaged and user */

        mmu_L1_table[i/BIGPAGE] = ent;
    }

    /* mapping for first range of device I/O */
    for(i=IO1_BASE; i<IO1_BASE+IO1_SIZE; i+=BIGPAGE)
    {
        uint32_t ent = (i&0xfff00000) | 2;
        ent |= (1<<2); /* bufferable (Device memory) */
        ent |= (1<<10) | (1<<4); /* R/W privlated, no access for user, no execute */

        mmu_L1_table[i/BIGPAGE] = ent;
    }
    /* 1MB pages leave a gap here */

    /* mapping for second range of device I/O */
    for(i=IO2_BASE; i<IO2_BASE+IO1_SIZE; i+=(1<<20))
    {
        uint32_t ent = (i&0xfff00000) | 2;
        ent |= (1<<2); /* bufferable (Device memory) */
        ent |= (1<<10) | (1<<4); /* R/W privlated, no access for user, no execute */

        mmu_L1_table[i/BIGPAGE] = ent;
    }
    /* 1MB pages leave a gap here */

    /* mapping for RAM (this page table lives here) */
    for(i=RAM_BASE; i<RAM_BASE+ramsize; i+=BIGPAGE)
    {
        uint32_t ent = (i&0xfff00000) | 2;
        ent |= (1<<16) | (1<<3) | (1<<2); /* sharable and cachable (Normal memory) */
        ent |= (3<<10); /* full access */

        mmu_L1_table[i/BIGPAGE] = ent;
    }
}

void mmu_setup(void)
{
    mmu_build_L1();

    {
        /* write TTBR0, set page table 0 base address */
        uint32_t base = (uint32_t)&mmu_L1_table;
        asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r"(base) : "memory" );
    }

    {
        /* write TTBCR, use page table 0 only */
        uint32_t val = (1<<4) ;
        asm volatile ("mcr p15, 0, %0, c2, c0, 2" : : "r"(val) : "memory" );
    }

    {
        /* write DACR domain access control register to allow all.
         * actual restrictions are in the page table
         */
        uint32_t val = 0xffffffff;
        asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(val) : "memory" );
    }

    /* enable MMU and sync*/
    {
        uint32_t ctrl;
        asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(ctrl):: "memory"); /* read SCTLR */
        ctrl |= (1<<0); /* MMU Enable */
        asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(ctrl): "memory"); /* read SCTLR */
    }

    asm volatile("isb\r\ndmb\r\ndsb" ::: "memory");
}

void mmu_test_access(uint32_t addr, unsigned mask, mmu_test_result *ret)
{
    uint32_t raw;

    memset(ret, 0, sizeof(*ret));
    ret->vaddr = addr;

    if(mask&MMU_PRIV) {
        if(mask&MMU_WRITE) {
            asm volatile ("mcr p15, 0, %0, c7, c8, 0" :: "r"(addr) : "memory"); /* write ATS1CPW */
        } else {
            asm volatile ("mcr p15, 0, %0, c7, c8, 2" :: "r"(addr) : "memory"); /* write ATS1CPR */
        }
    } else {
        if(mask&MMU_WRITE) {
            asm volatile ("mcr p15, 0, %0, c7, c8, 3" :: "r"(addr) : "memory"); /* write ATS1CUW */
        } else {
            asm volatile ("mcr p15, 0, %0, c7, c8, 2" :: "r"(addr) : "memory"); /* write ATS1CUR */
        }
    }
    asm volatile ("mrc p15, 0, %0, c7, c4, 0" :"=r"(raw) :: "memory"); /* read PAR */

    ret->f = raw&1;
    if(raw&1) {
        /* fault */
        ret->fault = (raw>>1)&BMASK(6-1);
    } else {
        ret->paddr = raw&BMASK2(31,12);
        ret->nos = !!(raw&(1<<10));
        ret->ns  = !!(raw&(1<<9));
        ret->sh  = !!(raw&(1<<7));
        ret->inner = (raw>>4)&BMASK(3);
        ret->outer = (raw>>2)&BMASK(2);
        ret->ss  = !!(raw&(1<<1));
    }
}

#define PBOOL(v) (v)?'T':'F'

void mmu_test_print(unsigned t, const mmu_test_result* res)
{
    printk(t, "Virtual: %x\n", (unsigned)res->vaddr);
    if(res->f) {
        printk(t, "Fault  : %x\n", res->fault);
    } else {
        printk(t, "PADDR: %x\n INN: %x\n OUT: %x\n",
               (unsigned)res->paddr, (unsigned)res->inner, (unsigned)res->outer);
        printk(t, " NOS: %c NS: %c SH: %c SS: %c\n",
               PBOOL(res->nos), PBOOL(res->ns),
               PBOOL(res->sh), PBOOL(res->ss));
    }
}
