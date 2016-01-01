/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"
#include "thread.h"
#include "systick.h"
#include "mmu.h"

thread_id T0, T1, T2;

void* thread1(void *user)
{
    assert(thread_current()==T1);
    printk(0, "running thread 1 %p\n", user);
    if(thread_resume(T2))
        printk(0, "failed to resume T2");
    return NULL;
}

void* thread2(void *user)
{
    assert(thread_current()==T2);
    printk(0, "running thread 2 %p\n", user);
    if(thread_resume(T0))
        printk(0, "failed to resume T0");
    return NULL;
}

void* thread3(void *user)
{
    printk(0, "enter thread 3 %p\n", user);
    thread_sleep(SYSTICK_RATE/2);
    printk(0, "leave thread 3 %p\n", user);
    return NULL;
}

static systick_cb fail_cb;
static uint32_t fail_cnt = SYSTICK_RATE*2;
static void failme(systick_cb *cb)
{
    (void)cb;
    if(--fail_cnt) return;
    printk(0, "Program timeout\n");
    halt();
}

void Init(void)
{
    thread_options opts;

    memset(&opts, 0, sizeof(opts));
    opts.prio = 1;

    mmu_setup();
    irq_setup();
    printk(0, "Setup page allocator\n");
    page_alloc_setup();
    printk(0, "Setup threading\n");
    thread_setup();
    systick_setup();

    T0 = thread_current();
    T1 = thread_create(&opts, &thread1, &thread1);
    T2 = thread_create(&opts, &thread2, &thread2);
    assert(thread_current()==T0);

    printk(0, "Init() is %u\n", T0);
    printk(0, "thread1() is %u %p\n", T1, &thread1);
    printk(0, "thread2() is %u %p\n", T2, &thread2);

    printk(0, "Begin round robin\n");

    printk(0, "current1 %u\n", thread_current());
    if(thread_resume(T1))
        printk(0, "failed to resume T1");
    printk(0, "current2 %u\n", thread_current());

    assert(thread_current()==T0);
    if(thread_suspend(T0))
        printk(0, "failed to suspend T0");
    printk(0, "current3 %u\n", thread_current());
    assert(thread_current()==T0);

    printk(0, "back in Init()\n\n");

    fail_cb.cb = &failme;
    systick_add(&fail_cb);

    T2 = thread_create(&opts, &thread3, &thread3);
    if(thread_resume(T2))
        printk(0, "failed to resume T3");

    printk(0, "Sleeping\n");
    thread_sleep(SYSTICK_RATE); /* 1 second */
    printk(0, "Awake\n");
}
