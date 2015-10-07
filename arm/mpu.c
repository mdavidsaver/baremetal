
#include "mmu.h"

//#define MMU_DEBUG

char __rom_start, __rom_end, __data_start;

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
#define SIZE_16MB  23
#define SIZE_512MB 29

/* Memory, sharable, cachable, write back */
#define TYPE_NORMAL 0b000111
/* MMIO, not sharable, not cachable */
#define TYPE_DEVICE 0b000001

/* PERM_<priv>_<user> */
#define PERM_NO_NO 0b000
#define PERM_RO_RO 0b110
#define PERM_RW_RW 0b011
#define PERM_RW_RO 0b010
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

    assert(size==0 || size>=32);

    assert(idx<num_regions);
    assert(perm<=0x7);
    assert(texscb<=0x3f);

    if(size>0) {
        unsigned tbits=log2_ceil(size)-1;
        uint32_t tbase = base, tsize = size;

        if(size<32) size=32;

#ifdef MMU_DEBUG
        printk(0, "MPU #%u\n\tRequest %x %x\n", idx,
               (unsigned)base, (unsigned)(base+size-1));
#endif

        while(tbits<32) {

            tsize = 1<<tbits;
            /* align base address down to size */
            tbase &= ~(tsize-1);

#ifdef MMU_DEBUG
            printk(0, " Try %u\n", tbits);
            printk(0, "\ttrial   %x %x\n",
                   (unsigned)tbase, (unsigned)(tbase+tsize-1));
#endif

            if(tbase+tsize<base+size) {
                /* oops, original request falls off the end */
                tbits+=1;
            } else {
                break; /* done */
            }
        }

        if(tbits>=32) {
            _assert_fail("MPU region would include entire VMA, but requested start address is not 0\n",
                         __FILE__, __LINE__);
        }

#ifdef MMU_DEBUG
        printk(0, "\tActual  %x %x\n",
               (unsigned)tbase, (unsigned)(tbase+tsize-1));
#endif

        base = tbase;

        if(!X) rasr |= 1<<(16+12);
        rasr |= perm<<(16+8);
        rasr |= texscb<<16;
        rasr |= (tbits-1)<<1;
        rasr |= 1; /* enable */

#ifdef MMU_DEBUG
        printk(0, "\tRASR %x\n",
               (unsigned)rasr);
#endif
    } else {
        base = 0;
#ifdef MMU_DEBUG
        printk(0, "MPU #%u Disable\n", idx);
#endif
    }

    out32((void*)0xe000ed98, idx);  /* RNR */
    out32((void*)0xe000eda0, 0);    /* RASR - disable mapping before modification */
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

    assert(num_regions>=5);

    /* Establish base mappings. */

    /* ROM, read-only for all, executable */
    set_region(0,
               (uint32_t)&__rom_start,
               (uint32_t)(&__rom_end-&__rom_start),
               TYPE_NORMAL, PERM_RO_RO, 1);

    /* RAM, read-only for user, writable for system */
    set_region(1, (uint32_t)&__data_start, RamSize, TYPE_NORMAL, PERM_RW_RO, 0);

    /* Periphrials, RW for system, no access for user */
    set_region(2, 0x40000000, 512*1024*1024, TYPE_DEVICE, PERM_RW_NO, 0);

    /* System, RW for system, no access for user */
    set_region(3, 0xe000e000, 4*1024, TYPE_DEVICE, PERM_RW_NO, 0);

    /* Mask off the lower 64 bytes to catch some *NULL errors */
    set_region(4, 0, 64, TYPE_NORMAL, PERM_NO_NO, 0);

    /* disable remainind regions for now */
    for(i=5; i<num_regions; i++)
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
