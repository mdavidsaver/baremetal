#include <stdarg.h>

#include "common.h"

static char hexchars[] = "0123456789ABCDEF";

void puthex(uint32_t v)
{
    uint8_t n = sizeof(v)*2;

    while(n--) {
        out8(A9_UART_BASE_1, hexchars[v>>28]);
        v<<=4;
    }
}

void putdec(int d)
{
    char buf[16];
    int s=d<0, m;
    unsigned p=sizeof(buf);

    while(d && p>0)
    {
        m = d%10;
        d /= 10;
        buf[--p] = hexchars[m];
    }
    if(s)
        buf[--p] = '-';

    while(p<sizeof(buf))
        out8(A9_UART_BASE_1, buf[p++]);
}

void vprintk(unsigned i, const char *fmt, va_list args)
{
    char c;
    (void)i;

    while( (c=*fmt++)!='\0')
    {
        if(c!='%') {
            out8(A9_UART_BASE_1, c);
            continue;
        }

        c = *fmt++;

        switch(c) {
        case 'd': {
            int v = va_arg(args, int);
            putdec(v);
        }
            break;
        case 'x': {
            unsigned v = va_arg(args, unsigned);
            puthex(v);
        }
            break;
        default:
            out8(A9_UART_BASE_1, '!');
            return;
        }
    }
}

void printk(unsigned i, const char *fmt, ...)
{
    va_list args; /* I don't have to figure out varargs, gcc to the rescue :) */

    va_start(args, fmt);
    vprintk(i, fmt, args);
    va_end(args);
}
