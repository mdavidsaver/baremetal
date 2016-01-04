
#include "kernel.h"
#include "process.h"
#include "user.h"

extern process *__sos_procs_start,
               *__sos_procs_end;

static
char idle_thread_stack[128];

static __attribute__((naked))
int idle_thread(const char *x)
{
    asm("idle_loop: wfi\n"
        " b idle_loop");
    asm("b halt"); // shouldn't get here
    return 0; // really shouldn't get here
}

static
process_config proc_internal_conf = {
    .super = 1,
    .name = "<kernel>",
};
static
process proc_internal = {
    .info = &proc_internal_conf,
};
static
thread_config thread_idle_config = {
    .name = "<idle>",
    .entry = &idle_thread,
    .prio = 3, // TODO: bounds check
    .stack = &idle_thread_stack,
    .stack_size = sizeof(idle_thread_stack),
};
static
thread thread_idle = {
    .info = &thread_idle_config,
    .proc = &proc_internal,
};
static
thread_config thread_main_config = {
    .name = "<main>",
};
static
thread thread_main = {
    .info = &thread_main_config,
    .proc = &proc_internal,
    .holdcount = 1,
    .active = 0,
};

void prepare_processes(void)
{
    process **S = &__sos_procs_start,
            **E = &__sos_procs_end;

    /* initially main is running, but has been stopped, idle is ready, as are any
     * process threads w/ autostart=1
     */
    thread_scheduler[0] = &thread_main;
    thread_start(&thread_idle);
    assert(thread_scheduler[1] == &thread_idle);

    for(;S<E; S++) {
        process *P = *S;
        printk("Prepare process '%s'\n", P->info->name);
#ifdef ARM7M
#endif
        if(!P->info->super) {
            uint32_t base, size, lsize, attr;
            // Prepare MPU configuration

            base = (uint32_t)P->info->memory_start;
            printk(" MPU Base: 0x%08x\n", (unsigned)base);
            assert((base&0x1f)==0);
            base |= 0x10 | MPU_USER_OFFSET;

            /* region size = 2**(lsize) */
            size = P->info->memory_end-P->info->memory_start;
            lsize = log2_floor(size);
            printk(" MPU Size: 0x%08x (2**%u)\n", (unsigned)size, (unsigned)lsize);
            assert((1u<<(lsize-1))==size);

            //TODO: use SRD to relax alignment requirements
            attr = 1 | ((lsize-1)<<1);

            /* RW, XN, sharable, cacheable, Normal */
            attr |= MPU_XN|MPU_AP_RWRW|MPU_RAM;
            attr |= (1<<28)|(3<<24)|(0<<19)|(1<<18)|(1<<17)|(0<<0);

            printk("MPU%u BASE=%08x SIZE=%08x ATTRS=%08x\n", MPU_USER_OFFSET, (unsigned)base, 1<<lsize, (unsigned)attr);
            P->mpu_settings[0] = base;
            P->mpu_settings[1] = attr;
            /* remaining regions disabled */
            P->mpu_settings[2] = 0x10 | (MPU_USER_OFFSET+1);
            P->mpu_settings[3] = 0;
            P->mpu_settings[4] = 0x10 | (MPU_USER_OFFSET+2);
            P->mpu_settings[5] = 0;
            P->mpu_settings[6] = 0x10 | (MPU_USER_OFFSET+3);
            P->mpu_settings[7] = 0;
        }
    }
}

void start_threading(void)
{
    process **S = &__sos_procs_start,
            **E = &__sos_procs_end;

    for(;S<E; S++) {
        process *P = *S;
        if(!P->info->autostart)
            return;
        process_start(P);
    }

    // main thread yields
#if defined(ARM7M)
    asm("svc 1");
#else
#error Not implemented
#endif
}
