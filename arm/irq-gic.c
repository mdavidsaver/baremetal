/* Handling of ARM Generic Interrupt Controller */
#include "common.h"

isrfunc irq_table[64];

void invalid_irq_vector(unsigned vect)
{
    printk(0, "Invalid/Unhandled IRQ on vector %x, disabling\n", vect);
    isr_disable(vect);
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

    out32(A9_PIC_CONF+0, 1); /* PIC Enable */

    asm volatile ("dmb\n cpsie i" ::: "memory");
}

int isr_install(unsigned vect, isrfunc fn)
{
    unsigned mask;
    int ret = 0;
    if(vect<32 || vect>=96)
        return 1;
    mask = irq_mask();
    if(irq_table[vect-32])
        ret = 1;
    else
        irq_table[vect-32] = fn;
    irq_unmask(mask);
    return ret;
}

int isr_enable(unsigned vect)
{
    unsigned mask, bit;
    if(vect>=96)
        return 1;
    bit = vect&0x1f;
    vect >>= 5; /* /= 32 */
    vect <<= 2; /* *= 4 */
    mask = irq_mask();
    out32(A9_PIC_CONF+0x100+vect, 1<<bit);
    irq_unmask(mask);
    return 0;
}

int isr_disable(unsigned vect)
{
    unsigned mask, bit;
    if(vect>=96)
        return 1;
    bit = vect&0x1f;
    vect >>= 5; /* /= 32 */
    vect <<= 2; /* *= 4 */
    mask = irq_mask();
    out32(A9_PIC_CONF+0x180+vect, 1<<bit);
    irq_unmask(mask);
    return 0;
}

void isr_dispatch(void)
{
    /* acknowledge/take responsibility for an active interrupt */
    unsigned vec = in32(A9_PIC_CPU_SELF+0x0c);

    if(vec<32 || vec>=96)
    {
        if(vec==0x3ff)
            return; /* ignore spurious */
        invalid_irq_vector(vec);
    } else
        irq_table[vec-32](vec);

    out32(A9_PIC_CPU_SELF+0x10, vec); /* EoI */
}

void irq_show(void)
{
    printk(0, "GIC State\n");
    printk(0, "RUN   %u\n", (unsigned)in32(A9_PIC_CPU_SELF+0x14));
    printk(0, "PEND  %x\n", (unsigned)in32(A9_PIC_CPU_SELF+0x18));
    printk(0, "ENA 0 %x\n", (unsigned)in32(A9_PIC_CONF+0x100));
    printk(0, "ENA 1 %x\n", (unsigned)in32(A9_PIC_CONF+0x104));
    printk(0, "ENA 2 %x\n", (unsigned)in32(A9_PIC_CONF+0x108));
    printk(0, "ACT 0 %x\n", (unsigned)in32(A9_PIC_CONF+0x200));
    printk(0, "ACT 1 %x\n", (unsigned)in32(A9_PIC_CONF+0x204));
    printk(0, "ACT 2 %x\n", (unsigned)in32(A9_PIC_CONF+0x208));
}
