/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"

static int done = 30;

static
void timerisr(unsigned v)
{
    int i;
    (void)v;
    out32(A9_TIMER_BASE_1+0x2C, 0xffffffff);
    i = __sync_sub_and_fetch(&done, 1);
    printk(0, "T%u\n", i);
}

void Init(void)
{
    printk(0, "hello\n");

    // first timer is ID34 in the pic
    isr_install(34, &timerisr); // disable ~ID34
    isr_enable(34);

    //out32(A9_PIC_CONF+0xf00, 0x2000001); // soft interrupt 1

    /* Load register */
    out32(A9_TIMER_BASE_1+0x20, 100000); /* 1MHz -> 10Hz */

    /* enable 32-bit periodic w/ irq, scale/1 */
    out32(A9_TIMER_BASE_1+0x28, 0b11100010);

    printk(0, "\nGo\n");
    {
        while(__sync_fetch_and_add(&done,0)>0) {}
    }
    printk(0, "\nDone\n");
}

