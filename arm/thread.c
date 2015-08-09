
#include "common.h"

typedef struct thread thread;

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
    void *stack;
    uint32_t registers[17]; /* r0-15 + cpsr */
    thread_state state;

    struct thread *next, *prev;
    thread_prio *prio;

    void *result;
    thread* joiner;
};

thread_prio all_thread_prio[4];

thread all_threads[16];

thread *current_thread;

typedef struct {
    unsigned prio;
    unsigned int user:1;
    unsigned int joinable:1;
} thread_options;

typedef unsigned thread_id;

typedef void*(*thread_func)(void*);

void _thread_schedule();

void _thread_cleanup_now(void);
void _thread_cleanup_joinable(void);
void _thread_free(thread *threadptr);

static void *idle_thread(void* unused)
{
    (void)unused;
    while(1)
        asm volatile ("wfi" ::: "memory");
    return NULL;
}

static
void thread_init_stack(thread_options* opt,
                       thread *threadptr,
                       thread_func fn,
                       void *user)
{

    memset(threadptr->registers, 0, sizeof(threadptr->registers));

    threadptr->registers[0]  = (uint32_t)user;
    threadptr->registers[13] = (uint32_t)threadptr->stack; /* SP */
    threadptr->registers[14] = (uint32_t)&_thread_cleanup_now; /* LR */
    threadptr->registers[15] = (uint32_t)fn; /* PC */
    threadptr->registers[16] = opt && opt->user ? 16 : 31; /* user or system mode */
    if(threadptr->registers[15]&1)
        threadptr->registers[16] |= (1<<5); /* user function is thumb */
}

void thread_setup(void)
{
    assert(current_thread==NULL);

    /* create implicit thread (for the caller)
     * and the idle thread
     */

    all_threads[0].stack = page_alloc();
    all_threads[1].stack = page_alloc();

    if(!all_threads[0].stack || !all_threads[1].stack) {
        printk(0, "thread_setup() out of memory\n");
        halt();
    }

    thread_init_stack(NULL, &all_threads[0], NULL, NULL);
    thread_init_stack(NULL, &all_threads[1], &idle_thread, NULL);

    all_thread_prio[0].first = &all_threads[0];
    all_thread_prio[0].last = &all_threads[1];
    all_threads[0].next = &all_threads[1];
    all_threads[1].prev = &all_threads[0];

    all_threads[0].state = thread_state_runable;
    all_threads[1].state = thread_state_runable;

    current_thread = &all_threads[0];

    asm volatile ("dsb" ::: "memory");

    /* don't call _thread_schedule() */
}

thread_id thread_create(thread_options* opt,
                        thread_func fn,
                        void *user)
{
    unsigned mask, i;
    unsigned prioidx = opt ? opt->prio : 0;
    void *stack = page_alloc();
    int threadidx = -1;
    thread *threadptr;

    if(!stack) return -1;
    if(prioidx>=NELEM(all_thread_prio)) goto error;

    memset(stack, 0, PAGE_SIZE);

    mask = irq_mask();

    for(i=0; i<NELEM(all_threads); i++)
    {
        thread *T = &all_threads[i];
        if(T->state!=thread_state_free)
            continue;
        threadidx = i;
        threadptr = T;
        T->state = thread_state_stopped;
    }

    irq_unmask(mask);

    if(threadidx==-1) goto error;

    threadptr->stack = stack;
    thread_init_stack(opt, threadptr, fn, user);

    threadptr->prio = &all_thread_prio[prioidx];

    mask = irq_mask();
    /* append to end of list */
    threadptr->prev = threadptr->prio->last;
    if(threadptr->prev)
        threadptr->prev->next = threadptr;
    threadptr->prio->last = threadptr;

    threadptr->state = thread_state_runable;
    irq_unmask(mask);

    _thread_schedule();

    return threadidx;
error:
    page_free(stack);
    return -1;
}

/* must be called with IRQ disabled.
 * Must not be called with sp set to the thread's stack
 */
void _thread_free(thread *threadptr)
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

    page_free(threadptr->stack);
    threadptr->prio = NULL;
    threadptr->state = thread_state_free;
}

int thread_suspend(unsigned threadidx)
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
        threadptr->state = thread_state_stopped;
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

int thread_resume(unsigned threadidx)
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
        threadptr->state = thread_state_runable;
        ret = 0;
        break;
    default:
        ret = -1;
    }

    _thread_schedule();

    irq_unmask(mask);

    return ret;
}

void *thread_join(unsigned threadidx)
{
    return NULL;
}
