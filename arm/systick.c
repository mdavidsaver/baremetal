#include "common.h"
#include "systick.h"

//#define SYSTICK_DEBUG

uint32_t systick;

static systick_cb *systick_head, *systick_tail;

#define SYSTICK_VECTOR 34

static
void systick_handler(unsigned vect)
{
    systick_cb *cur = systick_head, *next;
    out32(A9_TIMER_BASE_1+0x2C, 0xffffffff);
#ifdef SYSTICK_DEBUG
    printk(0, "systick_handler() %u %u\n", (unsigned)systick, (unsigned)systimer_get());
#endif
    (void)vect;
    __sync_fetch_and_add(&systick, 1);
    for(next=cur?cur->next:NULL;
        cur;
        cur=next, next=next?next->next:NULL)
    {
        (*cur->cb)(cur);
    }
}

uint32_t systick_get(void)
{
    return __sync_fetch_and_add(&systick, 0);
}

uint32_t systimer_get(void)
{
    return (SYSTIMER_RATE/SYSTICK_RATE)-in32(A9_TIMER_BASE_1+0x24);
}

void systick_setup(void)
{
    // first timer is ID34 in the pic
    if(isr_install(SYSTICK_VECTOR, &systick_handler))
        assert(0);

    /* Load register */
    out32(A9_TIMER_BASE_1+0x20, SYSTIMER_RATE/SYSTICK_RATE); /* 1MHz -> 10Hz */

    /* enable 32-bit periodic w/ irq, scale/1 */
    out32(A9_TIMER_BASE_1+0x28, 0b11100010);

    systick = 0;

    if(isr_enable(SYSTICK_VECTOR))
        assert(0);
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

    irq_unmask(mask);
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

    irq_unmask(mask);
    return 0;
}

void systimer_spin(uint32_t wait)
{
    uint32_t start = systimer_get(), cur;
    do {
        cur = systimer_get();
    } while((cur-start)<wait);
}
