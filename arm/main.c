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
    printk(0, "Enabling MMU\n");
    mmu_setup();
    printk(0, "Wow, still alive!\n");

    printk(0, "Read ROM %d\n", bar);
    printk(0, "Read RAM %d\n", foo);

    printk(0, "But not for long!\n");
    printk(0, "oops %x\n", (unsigned)in32((void*)0x30000000));
}
