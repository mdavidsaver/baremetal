
#include "common.h"
#include "uart.h"
#include "termout.h"

typedef struct {
    termdef T;
    char buf[128];
} term_uart;

static
int uart_tput(termdef *d, unsigned flags)
{
    d->buf[d->pos] = '\0';
    uart_puts(d->buf);
    d->pos = 0;
    return 0;
}

static
int uart_tflush(termdef *d)
{
    if(d->pos)
        uart_tput(d, 0);
    uart_flush();
    return 0;
}

static
term_uart term_uart0 = {
    .T = {
        .out = &uart_tput,
        .in = NULL,
        .flush = &uart_tflush,
        .len = 127,
        .pos = 0,
    },
};

termdef *term_k = &term_uart0.T;

int vprintk(const char *fmt, va_list args)
{
    int ret = term_vprintf(term_k, fmt, args);
    if(!ret)
        ret = term_flush(term_k, 0);
    return ret;
}

int printk(const char *fmt, ...)
{
    int ret;
    va_list args; /* I don't have to figure out varargs, gcc to the rescue :) */

    va_start(args, fmt);
    ret = vprintk(fmt, args);
    va_end(args);
    return ret;
}
