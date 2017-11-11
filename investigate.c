#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "tlb.h"
#include "mmio.h"

#define SHOW_SPR(NAME) ({uint32_t val = READ_SPR(NAME); \
    printk(#NAME " %08x\n", (unsigned)val); \
    val;})

static
void show_tlb0(void)
{
    SHOW_SPR(SPR_TLB0CFG);
    unsigned i;

    for(i=0; i<256; i++) {
        unsigned way;
        for(way=0; way<2; way++) {
            WRITE_SPR(SPR_MAS0, MAS0_TLB0| MAS0_ENT(way));
            WRITE_SPR(SPR_MAS2, MAS2_EPN(i<<12));
            __asm__ volatile ("tlbre" :::"memory");
            uint32_t mas0 = READ_SPR(SPR_MAS0),
                     mas1 = READ_SPR(SPR_MAS1),
                     mas2 = READ_SPR(SPR_MAS2),
                     mas3 = READ_SPR(SPR_MAS3);

            if(!(mas1&MAS1_V))
                continue;
            printk("TLB0 entry #%u.%u %08x %08x %08x %08x\n",
                i, way,
                (unsigned)mas0, (unsigned)mas1, (unsigned)mas2, (unsigned)mas3);
        }
    }
}

static
void show_tlb1(void)
{
    SHOW_SPR(SPR_TLB1CFG);
    unsigned i;

    for(i=0; i<16; i++) {
        WRITE_SPR(SPR_MAS0, MAS0_TLB1| MAS0_ENT(i));
        __asm__ volatile ("tlbre" :::"memory");
        uint32_t mas0 = READ_SPR(SPR_MAS0),
                 mas1 = READ_SPR(SPR_MAS1),
                 mas2 = READ_SPR(SPR_MAS2),
                 mas3 = READ_SPR(SPR_MAS3);

        if(!(mas1&MAS1_V))
            continue;
        printk("TLB1 entry #%u %08x %08x %08x %08x\n",
               i,
               (unsigned)mas0, (unsigned)mas1, (unsigned)mas2, (unsigned)mas3);
    }
}

/* DUART is 16550 compatible */
static
void show_uart(unsigned n)
{
    uint32_t base = CCSRBASE + (n==0 ? 0x4500 : 0x4600);
    uint8_t lcr, mcr, scratch;
    uint16_t div;

    lcr = in8x(base, 3);
    mcr = in8x(base, 4);
    scratch = in8x(base, 7);

    out8x(base, 3, lcr|0x80); // set DLAB
    div = in8x(base, 1); // MSB
    div<<=8;
    div|= in8x(base, 0); // LSB

    out8x(base, 3, lcr); // restore DLAB

    printk("UART%u LCR=%02x MCR=%02x UD=%04x scratch=%02x\n",
           n, lcr, mcr, div, scratch);
    printk(" Format %u%c%u\n",
           5+(lcr&3),
           (lcr&0x10) ? ((lcr&0x80) ? 'E' : 'O') : 'N',
           (lcr&0x4) ? 2 : 1);

    /* for 8500, baud rate is
     *   CCB Freq/(16*div)
     *  266e6/(16*1736) = 9576 (~9600)
     *  333e6/(16*2170) = 9591 (~9600)
     */
}

static
void show_ccsr(void)
{
    unsigned i;
    printk("CCSRBAR %08x %08x\n", (unsigned)CCSRBASE, (unsigned)in32x(CCSRBASE, 0));
    printk("PORPLLSR %08x\n", (unsigned)in32x(CCSRBASE, 0xe0000));
    printk("PORBMSR %08x\n", (unsigned)in32x(CCSRBASE, 0xe0004));
    printk("PORIMPSCR %08x\n", (unsigned)in32x(CCSRBASE, 0xe0008));
    printk("PORDEVSR %08x\n", (unsigned)in32x(CCSRBASE, 0xe000c));
    printk("PORDBGMSR %08x\n", (unsigned)in32x(CCSRBASE, 0xe0010));
    printk("PVR %08x\n", (unsigned)in32x(CCSRBASE, 0xe00a0));
    printk("SVR %08x\n", (unsigned)in32x(CCSRBASE, 0xe00a4));

    show_uart(0);
    show_uart(1);

    for(i=0; i<8; i++) {
        printk("LAWBAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0xc08, 0x20*i));
        printk("LAWAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0xc10, 0x20*i));
    }

    for(i=0; i<4; i++) {
        printk("CS%u_BNDS %08x\n", i, (unsigned)in32x(CCSRBASE+0x2000, 8*i));
        printk("CS%u_CONFIG %08x\n", i, (unsigned)in32x(CCSRBASE+0x2080, 4*i));
    }

    for(i=0; i<4; i++) {
        printk("LBC BR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8000, 8*i));
        printk("LBC OR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8004, 8*i));
    }
    
    for(i=0; i<4; i++) {
        printk("POTAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8C00, 0x20*i));
        printk("POTEAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8C04, 0x20*i));
        printk("POWBAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8C08, 0x20*i));
        printk("POWAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8C10, 0x20*i));
    }

    for(i=0; i<4; i++) {
        printk("PITAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8da0, 0x20*i));
        printk("PITEAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8da4, 0x20*i));
        printk("PIWBAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8da8, 0x20*i));
        printk("PIWAR%u %08x\n", i, (unsigned)in32x(CCSRBASE+0x8da0, 0x20*i));
    }
}

void Init(void)
{
    /* Short special registers */
    uint32_t msr, pvr;
    __asm__ ("mfmsr %0" : "=r"(msr));
    printk("MSR %08x\n", (unsigned)msr);
    SHOW_SPR(SPR_IVPR);
    pvr = SHOW_SPR(SPR_PVR);

    if((pvr&0xffff0000)==0x80200000) {
        printk("\ne500 core detected\n");
        uint32_t svr = SHOW_SPR(SPR_SVR);

        if((svr&0xffff0000)==0x80300000) {
            printk("\nmpc8540 detected\n");
            SHOW_SPR(SPR_HID0);
            SHOW_SPR(SPR_HID1);
            SHOW_SPR(SPR_MMUCFG);

            show_tlb0();
            show_tlb1();

            show_ccsr();

            /* assume mvme3100 */
            printk("Halt.\n");
            out8x(0xe2000000, 1, in8x(0xe2000000, 1) | 0xa0);
        }

    } else {
        printk("Unknown PPC core\n");
    }
    printk("Done\n");
    while(1) {}
}
