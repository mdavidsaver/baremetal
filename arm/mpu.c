
#include "mmu.h"

void memfault_handler(void)
{
    uint32_t sts  = in32((void*)0xe000ed28); /* CFSR */
    uint32_t addr = in32((void*)0xe000ed34); /* MMFAR */
    if(sts&(1<<7))
        printk(0, "Memory access fault accessing %x\n", (unsigned)addr);
    else
        printk(0, "Memory access fault accessing unknown\n");
    if(sts&(1<<1))
        printk(0, "  Data access violation\n");
    if(sts&(1<<3))
        printk(0, "  Derived fault on exception return");
    if(sts&(1<<4))
        printk(0, "  Derived fault on exception entry");
    halt();
}

static
unsigned num_regions;

#define SIZE_4K    12
#define SIZE_16MB  24
#define SIZE_512MB 29

#define TYPE_NORMAL 0b000111
#define TYPE_DEVICE 0b000001

/* Priv_User */
#define PERM_RO_RO 0b110
#define PERM_RW_RW 0b011
#define PERM_RW_NO 0b001

#ifdef __ARM_ARCH_7M__
static
void set_region(unsigned idx,
                uint32_t base,
                uint32_t size,
                unsigned texscb,
                unsigned perm,
                unsigned X
                )
{
    uint32_t rasr = 0;
    assert(idx<num_regions);
    assert((perm&~0x7)==0);
    assert((texscb&~0x3f)==0);
    assert((base&0x1f)==0);
    assert((size&~0x1f)==0);

    if(size>0) {
        if(!X) rasr |= 1<<(16+12);
        rasr |= perm<<(16+8);
        rasr |= texscb<<16;
        rasr |= size<<1;
        rasr |= 1; /* enable */
    }

    printk(0, "MPU #%u %x %x\n", idx,
           (unsigned)base, (unsigned)rasr);

    out32((void*)0xe000eda0, 0);    /* RASR - disable mapping before modification */
    out32((void*)0xe000ed98, idx);  /* RNR */
    out32((void*)0xe000ed9C, base); /* RBAR */
    out32((void*)0xe000eda0, rasr); /* RASR */
}
#endif /* __ARM_ARCH_7M__ */

void mmu_setup(void)
{
    unsigned int i;
#ifdef __ARM_ARCH_7M__
    num_regions = (in32((void*)0xe000ed90)>>8)&BMASK(15-8);
#endif

    if(!num_regions) {
        printk(0, "MPU not supported by this target\n");
        return;
    } else {
        printk(0, "MPU provides %u regions\n", num_regions);
    }

    assert(num_regions>=4);

    /* ROM, skip first page */
    set_region(0, 0x1000,     SIZE_16MB, TYPE_NORMAL, PERM_RO_RO, 1);
    /* RAM */
    set_region(1, 0x20000000, SIZE_16MB, TYPE_NORMAL, PERM_RW_RW, 0);
    /* Periphrials */
    set_region(2, 0x40000000, SIZE_512MB, TYPE_DEVICE, PERM_RW_NO, 0);
    /* System */
    set_region(3, 0xe000e000, SIZE_4K, TYPE_DEVICE, PERM_RW_NO, 0);

    for(i=4; i<num_regions; i++)
        set_region(i, 0, 0, 0, 0, 0);


#ifdef __ARM_ARCH_7M__
    out32((void*)0xe000ed94, 1); /* CTRL */
#endif
}

void mmu_test_access(uint32_t addr, unsigned mask, mmu_test_result *ret)
{
    (void)addr;
    (void)mask;
    (void)ret;
    printk(0, "Not implemented\n");
}

void mmu_test_print(unsigned t, const mmu_test_result* res)
{
    (void)t;
    (void)res;
}
