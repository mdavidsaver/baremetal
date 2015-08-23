/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"
#include "thread.h"

thread_id T0, T1, T2;

void* thread1(void *user)
{
    printk(0, "thread 1 %p\n", user);
    return NULL;
}

void* thread2(void *user)
{
    printk(0, "thread 1 %p\n", user);
    return NULL;
}

void Init(void)
{
    thread_options opts;

    opts.prio = 1;

    printk(0, "Setup threading\n");
    thread_setup();

    T0 = thread_current();
    T1 = thread_create(&opts, &thread1, &thread1);
    T2 = thread_create(&opts, &thread2, &thread2);

    printk(0, "main() is %u\n", T0);
    printk(0, "thread1() is %u\n", T1);
    printk(0, "thread2() is %u\n", T2);
}
