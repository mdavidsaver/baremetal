
#include "kernel.h"
#include "process.h"

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

static
void _proc_init(thread *T)
{
    process *proc = T->proc;
    const process_config *info = proc->info;

    /* TODO: race? */
    if(proc->initialized)
        return;
    proc->initialized = 1;

    memset(info->bss_start, 0, info->bss_end-info->bss_start);
    if(info->data_load!=info->data_start) {
        memcpy(info->data_start, info->data_load, info->data_end-info->data_start);
    }
    //TODO: global ctors
}

static
void _thread_start(thread *T, const char *cmd)
{
    int ret;
    _proc_init(T);
    ret = (T->info->entry)(cmd);
    (void)ret; // todo
    T->active = 0;
    asm("dsb\n" "svc 1"); // yield w/ !active destroy thread
}

static void thread_prepare(thread *T)
{
#if defined(ARM7M)
    uint32_t *frame;
    T->frame = T->info->stack_size+(char*)T->info->stack;
    T->frame -= 4*7;

    frame = (uint32_t*)T->frame;
    frame[FRAME_R0] = (uint32_t)T;
    frame[FRAME_LR] = (uint32_t)&halt;
    /* the unstacked PC shouldn't have the LSB set, thumb state is carried in the PSR */
    frame[FRAME_PC] = ~1&(uint32_t)&_thread_start;
    frame[FRAME_PSR]= (1<<24); // always thumb state
#else
#error Not implemented
#endif
    T->holdcount = 1;
    T->active = 0;
}

void prepare_processes(void)
{
    process **S = &__sos_procs_start,
            **E = &__sos_procs_end;

    /* initially main is running, but has been stopped, idle is ready, as are any
     * process threads w/ autostart=1
     */
    thread_scheduler[0] = &thread_main;
    thread_prepare(&thread_idle);
    thread_idle.active = 1;
    thread_resume(&thread_idle);
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

        {
            thread *T = P->info->threads;
            for(;T->info; T++) {
                printk(" Thread: '%s'\n", T->info->name);

                memset(&T->schednode, 0, sizeof(T->schednode));
                T->holdcount = 0;
                T->active = 0;
                T->frame = T->info->stack;

                thread_prepare(T);

                if(T->info->autostart) {
                    printk("  Autostart\n");
                    T->active = 1;
                    T->holdcount = 1;
                    thread_resume(T);
                }
            }
        }
    }

    // main thread yields
#if defined(ARM7M)
    asm("svc 1");
#else
#error Not implemented
#endif
}
