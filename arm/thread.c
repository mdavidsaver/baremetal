
#include "common.h"
#include "thread.h"

typedef struct {
    thread *first, *last;
} thread_prio;

typedef enum {
    thread_state_free,
    thread_state_stopped,
    thread_state_runable,
    thread_state_complete
} thread_state;

struct thread {
    /* register array must be first */
    uint32_t registers[12]; /* cspr, r4-r14 (r4-r12,sp,lr) */

    void *stack;
    thread_state state;
    unsigned int suspend_count;

    struct thread *next, *prev;
    thread_prio *prio;

    void *result;
    thread* joiner;
};

thread_prio all_thread_prio[4];

thread all_threads[16];

thread *current_thread;

void _thread_switch(thread* from, thread* to);
void _thread_run(thread* t);
void _thread_start(void);

void _thread_schedule(void);
void _thread_cleanup_now(void);
void _thread_cleanup_joinable(void);
void _thread_free(thread *threadptr);

int thread_resume(unsigned threadidx);

static void *_idle_thread(void* unused)
{
    (void)unused;
    while(1)
        asm volatile ("wfi" ::: "memory");
    return NULL;
}

static
void thread_init_context(thread_options* opt,
                       thread *threadptr,
                       thread_func fn,
                       void *user)
{

    memset(threadptr->registers, 0, sizeof(threadptr->registers));

#define CPSR (0)
#define REG(N) ((N)+1-4)

    threadptr->registers[REG(4)] = (uint32_t)fn;
    threadptr->registers[REG(5)] = (uint32_t)user;
    threadptr->registers[REG(13)] = (uint32_t)threadptr->stack; /* SP */
    threadptr->registers[REG(14)] = (uint32_t)&_thread_start; /* LR */

    threadptr->registers[CPSR] = opt && opt->user ? 16 : 31; /* user or system mode */
    if(threadptr->registers[REG(4)]&1)
        threadptr->registers[CPSR] |= (1<<5); /* _thread_start is thumb? */

#undef CPSR
#undef REG
}

/* interrupts must be disabled */
static
void thread_list_append(thread *threadptr)
{
    /* append to end of list */
    threadptr->prev = threadptr->prio->last;
    if(threadptr->prev)
        threadptr->prev->next = threadptr;
    threadptr->prio->last = threadptr;
    if(!threadptr->prio->first)
        threadptr->prio->first = threadptr;
}

static
void thread_list_remove(thread *threadptr)
{
    thread_prio *prio = threadptr->prio;

    if(threadptr->next)
        threadptr->next->prev = threadptr->prev;
    if(threadptr->prev)
        threadptr->prev->next = threadptr->prev;
    if(threadptr == prio->first)
        prio->first = threadptr->next;
    if(threadptr == prio->last)
        prio->last = threadptr->prev;
    if(threadptr == current_thread)
        current_thread = NULL;
}

void thread_setup(void)
{
    unsigned mask;
    assert(current_thread==NULL);

    /* create implicit thread (for the caller)
     * and the idle thread
     */

    mask = irq_mask();

    all_threads[0].stack = page_alloc();
    all_threads[1].stack = page_alloc();

    if(!all_threads[0].stack || !all_threads[1].stack) {
        printk(0, "thread_setup() out of memory\n");
        halt();
    }

    all_threads[0].prio = &all_thread_prio[0]; /* implicit thread has highest prio */
    all_threads[1].prio = &all_thread_prio[NELEM(all_thread_prio)-1]; /* idle has lowest prio */

    thread_init_context(NULL, &all_threads[0], NULL, NULL);
    thread_init_context(NULL, &all_threads[1], &_idle_thread, NULL);

    thread_list_append(&all_threads[0]);
    thread_list_append(&all_threads[1]);

    all_threads[0].state = thread_state_runable;
    all_threads[1].state = thread_state_runable;

    current_thread = &all_threads[0];

    asm volatile ("dsb" ::: "memory");

    /* don't call _thread_schedule() */
    irq_unmask(mask);
}

thread_id thread_current(void)
{
    thread *cur;
    unsigned mask;

    mask = irq_mask();
    cur = current_thread;
    irq_unmask(mask);

    if(!cur) return INVALID_THREAD;

    return (cur-&all_threads[0])/sizeof(*cur);
}

thread_id thread_create(thread_options* opt,
                        thread_func fn,
                        void *user)
{
    unsigned mask, i;
    unsigned prioidx = opt ? opt->prio : 0;
    void *stack = page_alloc();
    thread_id threadidx = INVALID_THREAD;
    thread *threadptr;

    if(!stack) return INVALID_THREAD;
    if(prioidx>=NELEM(all_thread_prio)) goto error;

    mask = irq_mask();

    /* search for unallocated thread */
    for(i=0; i<NELEM(all_threads); i++)
    {
        thread *T = &all_threads[i];
        if(T->state==thread_state_free)
        {
            threadidx = i;
            threadptr = T;
            T->state = thread_state_stopped;
            T->suspend_count = 1;
            break;
        }
    }

    irq_unmask(mask);

    if(threadidx==INVALID_THREAD) goto error;

    threadptr->stack = stack;
    thread_init_context(opt, threadptr, fn, user);

    threadptr->prio = &all_thread_prio[prioidx];

    return threadidx;
error:
    page_free(stack);
    return INVALID_THREAD;
}

/* must be called with IRQ disabled.
 * Must not be called with sp set to the thread's stack
 */
void _thread_free(thread *threadptr)
{
    if(threadptr->state==thread_state_runable)
        thread_list_remove(threadptr);
    threadptr->state = thread_state_stopped;

    page_free(threadptr->stack);
    threadptr->prio = NULL;
    threadptr->state = thread_state_free;
}

int thread_suspend(thread_id threadidx)
{
    unsigned mask;
    int ret;
    thread *threadptr;

    if(threadidx>NELEM(all_threads)) return -1;
    threadptr = &all_threads[threadidx];

    mask = irq_mask();
    switch(threadptr->state)
    {
    case thread_state_runable:
        assert(threadptr->suspend_count==0);
        thread_list_remove(threadptr);
        threadptr->suspend_count = 1;
        threadptr->state = thread_state_stopped;
        ret = 0;
        break;
    case thread_state_stopped:
        assert(threadptr->suspend_count>0);
        threadptr->suspend_count++;
        ret = 0;
        break;
    default:
        ret = -1;
    }

    if(threadptr==current_thread)
        _thread_schedule();

    irq_unmask(mask);

    return ret;
}

int thread_resume(thread_id threadidx)
{
    unsigned mask;
    int ret;
    thread *threadptr;

    if(threadidx>NELEM(all_threads)) return -1;
    threadptr = &all_threads[threadidx];

    mask = irq_mask();
    switch(threadptr->state)
    {
    case thread_state_stopped:
        assert(threadptr->suspend_count>0);
        if(0==--threadptr->suspend_count) {
            thread_list_append(threadptr);
            threadptr->state = thread_state_runable;
        }
        ret = 0;
        break;
    default:
        ret = -1;
    }

    assert(current_thread);

    if(threadptr->prio < current_thread->prio)
        _thread_schedule();

    irq_unmask(mask);

    return ret;
}

static
thread* thread_schedule_next(void)
{
    unsigned i;
    thread *next = NULL;

    for(i=0; i<NELEM(all_thread_prio); i++)
    {
        thread *threadptr = all_thread_prio[i].first;
        if(threadptr && threadptr->state==thread_state_runable) {
            next = threadptr;
            break;
        }
    }
    assert(next);
    return next;
}

void _thread_schedule(void)
{
    thread *cur = current_thread,
           *next = thread_schedule_next();
    if(cur==next)
        return;
    current_thread = next;
    _thread_switch(cur, next);
}

void _thread_cleanup_now(void)
{
    thread *next;
    /* TODO joinable */
    _thread_free(current_thread);
    next = current_thread = thread_schedule_next();
    _thread_run(next);
}

void *thread_join(unsigned threadidx)
{
    (void)threadidx;
    return NULL;
}
