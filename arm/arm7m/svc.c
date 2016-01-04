
#include "kernel.h"
#include "process.h"
#include "uart.h"
#include "systick.h"

extern process *__sos_procs_start,
               *__sos_procs_end;

static inline __attribute__((always_inline))
uint32_t user_in32(const void *addr)
{
    uint32_t ret;
    __asm__("ldrt %0, [%1]" : "=r"(ret) : "r"(addr));
    return ret;
}

static inline __attribute__((always_inline))
uint8_t user_in8(const void *addr)
{
    uint8_t ret;
    __asm__("ldrbt %0, [%1]" : "=r"(ret) : "r"(addr));
    return ret;
}

static inline
int strncpy_fromuser(char *kbuf, const char *ubuf, unsigned kbuflen)
{
    char c;
    char *kbufE = kbuf+kbuflen;
    while((c=user_in8(ubuf++))!='\0' && kbuf<kbufE) {
        *kbuf++ = c;
    }
    if(kbuf==kbufE)
        return -1;
    *kbuf='\0';
    return 0;
}

typedef int (*svc_fn)(uint32_t *frame);

/* really thread exit */
static
int svc_halt(uint32_t *frame)
{
    unsigned alive = 0;
    thread *T = thread_scheduler[0], *N;
    process *P = T->proc;
    assert(T->active && P->running);

    thread_suspend(T);
    T->active = 0;

    for(N=P->info->threads; N->info; N++) {
        alive |= N->active;
    }

    if(alive) {
        // there is at least one more thread active in this process
        // nothing more to do
        return 0;
    } else if(P->initialized) {
        // this was the last thread in the process, it now becomes the cleaner
        process_cleanup(T);
        thread_resume(T);
        // the cleaner will clear 'initialized' then halt again
        return 0;
    } else {
        // the cleaner completed
        P->running = 0;
    }

    // any processes still running?
    {
        alive = 0;
        process **S = &__sos_procs_start,
                **E = &__sos_procs_end;
        for(;S<E; S++) {
            P = *S;
            alive |= P->running;
        }
    }

    if(!alive)
        halt();
    return 0;
}

/* force re-scheduling, allow another equal or higher priority thread to run */
static
int svc_yield(uint32_t *frame)
{
    thread_yield(thread_scheduler[0]);
    return 0;
}

/* interact with the debug terminal */
static
int svc_uart(uint32_t *frame)
{
    int nput = 0;
    char c;
    char *buf = (char*)frame[FRAME_R0];
    uint32_t flags = frame[FRAME_R1];
    if(buf) {
        while((c=user_in8(buf++))!='\0') {
            uart_putc(c);
            nput++;
        }
    }
    if(flags&1) {
        uart_flush();
    }
    return nput;
}

static
void _svc_sleep_tick(systick_cb *cb)
{
    thread *T = container(cb, thread, waiter);
    if(--T->timeout_ticks)
        return;
    systick_del(&T->waiter);
    thread_resume(T);
}

/* suspend a thread for a fixed (minimum) time */
static
int svc_sleep(uint32_t *frame)
{
    uint32_t val = frame[FRAME_R0],
             flags= frame[FRAME_R1];
    (void)flags;
    thread *T = thread_scheduler[0];

    if(val==0)
        return 0;

    T->timeout_ticks = val;
    T->waiter.cb = &_svc_sleep_tick;
    systick_add(&T->waiter);
    thread_suspend(T);
    return 0;
}

/* spawn a new thread or process */
static
int svc_spawn(uint32_t *frame)
{
    int ret;
    char nbuf[16];
    char *name = (char*)frame[FRAME_R0];
    uint32_t flags = frame[FRAME_R1];
    thread *T = thread_scheduler[0];

    if((ret=strncpy_fromuser(nbuf, name, sizeof(nbuf)))!=0)
        return ret;

    if(flags&1) { // new thread in current process
        process *P = T->proc;
        thread *N;

        for(N=P->info->threads; N->info; N++) {
            if(strcmp(nbuf, N->info->name)!=0)
                continue;
            if(N->active)
                return -1;
            return thread_start(N);
        }

    }

    return -1;
}

static
svc_fn svc_calls[] = {
    &svc_halt, // 0
    &svc_yield,
    &svc_uart,
    &svc_sleep,
    &svc_spawn, // 4
};

void svc_handler_c(uint32_t *frame)
{
    unsigned ret = -127; // bad SVC
    uint16_t *pc, inst;
    uint8_t idx;

    pc = (void*)frame[FRAME_PC];
    inst = *(pc-1);

    assert((inst&0xff00)==0xdf00);

    idx = inst&0xf;
    if(idx<NELEM(svc_calls))
        ret = svc_calls[idx](frame);
    frame[FRAME_R0] = ret;
}
