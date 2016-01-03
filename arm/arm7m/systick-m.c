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

static systick_cb *systick_head, *systick_tail;

void systick_handler_c(void)
{
    systick_cb *cur = systick_head, *next;

#ifdef SYSTICK_DEBUG
    printk("systick_handler() %u\n", (unsigned)systick);
#endif
    __sync_fetch_and_add(&systick, 1);
    for(next=cur?cur->next:NULL;
        cur;
        cur=next, next=next?next->next:NULL)
    {
        (*cur->cb)(cur);
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

    scs_out32(RVR, cal);

    scs_out32(CSR, 0x3); /* enable timer and IRQ */
}

uint32_t systick_get(void)
{
    return __sync_fetch_and_add(&systick, 0);
}

int systick_add(struct systick_cb* T)
{
    unsigned mask = irq_mask();

    /* append to list */
    T->next = NULL;
    T->prev = systick_tail;
    if(T->prev)
        T->prev->next = T;
    systick_tail = T;

    if(!systick_head) {
        systick_head = T;
    }

    irq_restore(mask);
    return 0;
}

int systick_del(struct systick_cb* T)
{
    unsigned mask = irq_mask();

    if(T->prev)
        T->prev->next = T->next;
    else
        systick_head = T->next;

    if(T->next)
        T->next->prev = T->prev;
    else
        systick_tail = T->prev;

    irq_restore(mask);
    return 0;
}

void systick_spin(uint32_t wait)
{
    uint32_t start = systick_get(), cur;
    do {
        cur = systick_get();
    } while((cur-start)<wait);
}
