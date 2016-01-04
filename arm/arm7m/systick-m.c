/* System tick using the armv7-m systick timer
 */

#include "bsp.h"
#include "kernel.h"
#include "systick.h"

//#define SYSTICK_DEBUG

#define SYSTIMER_RATE 100 /* Hz */

#define CSR  (0x010u)
#define RVR  (0x014u)
#define CVR  (0x018u)
#define CAL  (0x01cu)

volatile uint32_t systick;

static
ELLLIST systick_actions;

void systick_handler_c(void)
{
    ELLNODE *cur, *next;

#ifdef SYSTICK_DEBUG
    printk("systick_handler() %u\n", (unsigned)systick);
#endif
    __sync_fetch_and_add(&systick, 1);
    foreach_ell_safe(cur, next, &systick_actions)
    {
        systick_cb *cb = container(cur, systick_cb, node);
        (*cb->cb)(cb);
    }
}

void systick_setup(void)
{
    uint32_t cal = scs_in32(CAL);

    if(cal&0x40000000)
        printk("SYSTICK clock skew\n");

    cal &= BMASK(23); /* 100 Hz value */
    if(cal==0) {
        printk("No timer calibration, using a default\n");
        cal = 10000; /* default for QEMU */
    }

    cal *= SYSTIMER_RATE/SYSTICK_RATE;
    cal &= BMASK(23);

    if(cal==0) {
        printk("Invalid timer calibration and/or tick rate, using a default\n");
        cal = 100000; /* 10Hz for QEMU */
    }

    printk("systick CAL = %08x\n", (unsigned)cal);
    scs_out32(RVR, cal);

    scs_out32(CSR, 0x1); /* enable timer and disable IRQ */
}

uint32_t systick_get(void)
{
    return __sync_fetch_and_add(&systick, 0);
}

int systick_add(struct systick_cb* T)
{
    unsigned mask = irq_mask();

    ellPushBack(&systick_actions, &T->node);

    scs_out32(CSR, 0x3); /* enable timer and IRQ */
    irq_restore(mask);
    return 0;
}

int systick_del(struct systick_cb* T)
{
    unsigned mask = irq_mask();

    ellRemove(&systick_actions, &T->node);

    if(!ellFirst(&systick_actions))
        scs_out32(CSR, 0x1); /* enable timer and disable IRQ */
    irq_restore(mask);
    return 0;
}
