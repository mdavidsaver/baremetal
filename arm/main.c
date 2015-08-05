/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"
#include "mmu.h"

static volatile int foo = -424242; /* I'm in ram */

static volatile const int bar = -242424; /* I'm in ROM */

void Init(void)
{
    putdec(0);
    putdec(1);
    putdec(-1);
    putchar('\n');
    putdec(10);
    putdec(-10);
    printk(0, "Init()\n");
    mmu_setup();
    printk(0, "Wow, still alive!\n");

    printk(0, "Read ROM %d\n", bar);
    printk(0, "Read RAM %d\n", foo);
}
