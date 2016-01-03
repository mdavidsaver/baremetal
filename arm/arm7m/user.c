
#include "common.h"
#include "termout.h"
#include "user.h"

#define SYSCALL(N, NAME, ...) \
    int __attribute__((naked)) sys_##NAME (__VA_ARGS__) { asm("swi " #N); }
#include "syscalls.h"

typedef struct {
    termdef term;
    char buf[128];
} term_svc_t;

static
int term_svc_out(termdef *d, unsigned flags)
{
    int ret = 0;
    if(d->pos) {
        assert(d->pos<127);
        d->buf[d->pos] = '\0';
        ret = sys_uart(d->buf, flags&1);
        if(ret<=0 || (unsigned)ret==d->pos) {
            // error or completely consumed
            d->pos = 0;
        } else {
            memmove(d->buf, d->buf+(unsigned)ret, d->pos-(unsigned)ret);
            d->pos -= ret;
        }
    }
    return 0;
}

static
int term_svc_flush(termdef *d)
{
    term_svc_out(d, 1);
}

static
term_svc_t term_svc = {
    .term = {
        .out = &term_svc_out,
        .flush = &term_svc_flush,
        .len = 127,
        .pos = 0
    },
};

int vprintf(const char *fmt, va_list args)
{
    return term_vprintf(&term_svc, fmt, args);
}

int printf(const char *fmt, ...)
{
    int ret;
    va_list args; /* I don't have to figure out varargs, gcc to the rescue :) */

    va_start(args, fmt);
    ret = term_vprintf(&term_svc, fmt, args);
    va_end(args);
    return ret;
}
