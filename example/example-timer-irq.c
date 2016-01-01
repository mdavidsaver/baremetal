/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "bsp.h"
#include "common.h"
#include "systick.h"

static int done = 30;

static systick_cb cb;

static
void timertick(systick_cb *X)
{
    int i;
    (void)X;
    i = __sync_sub_and_fetch(&done, 1);
    printk(0, "T%u\n", i);
}

void Init(void)
{
    irq_setup();
    systick_setup();
    printk(0, "hello\n");

    cb.cb = &timertick;
    systick_add(&cb);

    irq_show();

    assert(done>0);
    printk(0, "\nGo\n");
    {
        while(__sync_fetch_and_add(&done,0)>0) {}
    }
    printk(0, "\nDone\n");
}

