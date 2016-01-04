
#include "process.h"
#include "kernel.h"
#include "user.h"
#include "io.h"

static
ELLLIST thread_by_prio[4];
static
volatile process *process_current; // the process w/ MPU config loaded

/* slow path for context switch.
 * if we arrive here then we know there will be a context switch.
 * the current thread's registers r4-r12 have already been stacked.
 * This call should return a value for LR in an exception handler (not nested)
 * to return to the next process.
 */
uint32_t thread_do_switch(uint32_t *frame)
{
    uint32_t ret = 0xfffffffd; // always return to thread mode
    thread *cur = thread_scheduler[0],
           *next= thread_scheduler[1];
    process *pnext= next->proc;
    uint32_t control = !pnext->info->super;

    cur->frame = (char*)frame;

    //TODO: check cur->active and do something?

    if(process_current!=pnext && control) {
        // switching to a different non-privlaged process

        uint32_t mpurbar = M_SYS_BASE + 0xd9c; // MPU_RBAR0
        // load 8 32-bit words and write them to the 4 MPU region alias registers
        __asm__ ("ldmia %[thread], {r0-r6,r8}\n"
                 "stmia %[mpu], {r0-r6,r8}"
            :: [thread]"r"(pnext->mpu_settings),
               [mpu]"r"(mpurbar)
            : "r0",  "r1",  "r2",  "r3",
              "r4",  "r5",  "r6",  "r8"
        );

        process_current = pnext;
    } else {
        /* another thread in the same process, or a supervisor thread.
         * Leave the MPU alone.
         */
    }

    __asm__ volatile ("msr CONTROL, %0" :: "r"(control):); // update priv/unpriv
    __asm__ volatile ("msr PSP, %0" :: "r"(next->frame):);

    thread_scheduler[0] = next;

    return ret;
}

int process_start(process *P)
{
    thread *T;
    if(P->running)
        return -1;
    for(T=P->info->threads; T->info; T++)
    {
        if(!T->info->proc_main)
            continue;
        thread_start(T);
        P->running = 1;
        return 0;
    }
    return -2; // no entry point thread, this should be trapped at config time
}

static
void _thread_return(void)
{
    sys_halt();
}

static
void _proc_cleaner(thread *T, const char *cmd)
{
    // TODO: run dtors here
    T->proc->initialized = 0;
    sys_halt();
}

void process_cleanup(thread *T)
{
    assert(!T->active);;
#if defined(ARM7M)

    uint32_t *frame;
    T->frame = T->info->stack_size+(char*)T->info->stack;
    T->frame -= 4*7;

    frame = (uint32_t*)T->frame;
    frame[FRAME_R0] = (uint32_t)T;
    frame[FRAME_LR] = ~1&(uint32_t)&_thread_return;
    /* the unstacked PC shouldn't have the LSB set, thumb state is carried in the PSR */
    frame[FRAME_PC] = ~1&(uint32_t)&_proc_cleaner;
    frame[FRAME_PSR]= (1<<24); // always thumb state

    /* we are re-setting the stack w/o changing the current thread
     * so thread_switch() will be a no-op.
     * So update PSP directly
     */
    __asm__ volatile ("msr PSP, %0" :: "r"(frame):);
#else
#error Not implemented
#endif
    T->holdcount = 1;
    T->active = 1;
}

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
    sys_halt();
}

int thread_start(thread *T)
{
    uint32_t *frame;

    if(T->active)
        return -1;

    T->frame = T->info->stack_size+(char*)T->info->stack;
    T->frame -= 4*7;

    frame = (uint32_t*)T->frame;
    frame[FRAME_R0] = (uint32_t)T;
    frame[FRAME_LR] = ~1&(uint32_t)&_thread_return; // TODO: really clear IM bit for LR????
    /* the unstacked PC shouldn't have the LSB set, thumb state is carried in the PSR */
    frame[FRAME_PC] = ~1&(uint32_t)&_thread_start;
    frame[FRAME_PSR]= (1<<24); // always thumb state

    T->active = 1;
    T->holdcount = 1;
    thread_resume(T);
    return 0;
}

void
thread_resume(thread *T)
{
    uint8_t prio = T->info->prio;
    thread *pending = (thread*)thread_scheduler[1];
    assert(T->holdcount && T->active);
    T->holdcount--;

    if(T->holdcount)
        return;

    if(prio>=NELEM(thread_by_prio))
        prio = NELEM(thread_by_prio);

    ellPushBack(&thread_by_prio[prio], &T->schednode);

    /* we have higher priority, take over */
    if(!pending || prio < pending->info->prio)
        thread_scheduler[1] = T;
}

static void thread_schedule_next(void)
{
    thread *next = NULL;
    unsigned i;

    for(i=0; i<NELEM(thread_by_prio); i++) {
        ELLNODE *node = ellFirst(&thread_by_prio[i]);
        if(!node)
            continue;
        next = container(node, thread, schednode);
        assert(next->active);
        assert(next->holdcount==0);
        break;
    }
    assert(next);
    thread_scheduler[1] = next;
}

void thread_suspend(thread *T)
{
    uint8_t prio = T->info->prio;
    if(T->holdcount==0) {
        ellRemove(&thread_by_prio[prio], &T->schednode);
    }
    T->holdcount++;

    thread_schedule_next();
}

void thread_yield(thread *T)
{
    uint8_t prio = T->info->prio;

    if(T->active) {
        assert(T->holdcount==0);
        ellRemove(&thread_by_prio[prio], &T->schednode);
        ellPushBack(&thread_by_prio[prio], &T->schednode);
    }
    thread_schedule_next();
}

#undef thread_scheduler
thread *thread_scheduler[2]; /* {current, next} */
