/* Handling of ARM Global Interrupt Controller */
#include "common.h"

isrfunc irq_table[64];

void invalid_irq_vector(unsigned vect)
{
    printk(0, "Invalid IRQ vector %x\n", vect);
    halt();
}

void irq_setup(void)
{
    unsigned i;

    for(i=0; i<NELEM(irq_table); i++)
        irq_table[i] = &invalid_irq_vector;

    out32(A9_PIC_CPU_SELF+4, 0xff); /* unmask all */
    out32(A9_PIC_CPU_SELF+0, 1); // enable for this CPU

    /* disable all */
    out32(A9_PIC_CONF+0x184, ~0u);
    out32(A9_PIC_CONF+0x188, ~0u);

    /* clear any pending */
    out32(A9_PIC_CONF+0x284, ~0u);
    out32(A9_PIC_CONF+0x288, ~0u);

    out32(A9_PIC_CONF+0x104, 0x0000004);
    out32(A9_PIC_CONF+0, 1); /* PIC Enable */

    asm volatile ("dmb\n cpsie i" ::: "memory");
}

int isr_install(unsigned vect, isrfunc fn)
{
    unsigned mask;
    if(vect<32 || vect>=96)
        return 1;
    mask = irq_mask();
    irq_table[vect-32] = fn;
    irq_unmask(mask);
    return 0;
}

int isr_enable(unsigned vect)
{
    unsigned mask, bit;
    if(vect<32 || vect>=96)
        return 1;
    vect -= 32;
    bit = vect&0x1ff;
    vect >>= 5; /* /= 32 */
    vect <<= 2; /* *= 4 */
    mask = irq_mask();
    out32(A9_PIC_CONF+0x104+vect, 1<<bit);
    irq_unmask(mask);
    return 0;
}

int isr_disable(unsigned vect)
{
    unsigned mask, bit;
    if(vect<32 || vect>=96)
        return 1;
    vect -= 32;
    bit = vect&0x1ff;
    vect >>= 5; /* /= 32 */
    vect <<= 2; /* *= 4 */
    mask = irq_mask();
    out32(A9_PIC_CONF+0x184+vect, 1<<bit);
    irq_unmask(mask);
    return 0;
}

void isr_dispatch(void)
{
    /* acknowledge/take responsibility for an active interrupt */
    unsigned vec = in32(A9_PIC_CPU_SELF);

    if(vec<32 || vec>=96)
    {
        if(vec==0x3ff)
            return; /* ignore spurious */
        invalid_irq_vector(vec);
        return;
    }

    irq_table[vec-32](vec);

    out32(A9_PIC_CPU_SELF+0x10, vec); /* IACK */
}
