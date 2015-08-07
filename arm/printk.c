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

static
void putdecnum(unsigned d, int neg)
{
    char buf[16];
    int m;
    unsigned p=sizeof(buf);
    buf[--p]='\0';

    while(d && p>0)
    {
        m = d%10;
        d /= 10;
        buf[--p] = hexchars[m];
    }
    if(neg)
        buf[--p] = '-';
    if(p==sizeof(buf)-1)
        buf[--p] = '0';

    while(p<sizeof(buf))
        out8(A9_UART_BASE_1, buf[p++]);
}

void putdec(int d)
{
    int s=d<0;
    if(s) d = -d;
    putdecnum(d, s);
}

void putudec(unsigned v)
{
    putdecnum(v, 0);
}

void putchar(char c)
{
    out8(A9_UART_BASE_1, c);
}

void vprintk(unsigned i, const char *fmt, va_list args)
{
    char c;
    (void)i;

    while( (c=*fmt++)!='\0')
    {
        if(c!='%') {
            putchar(c);
            continue;
        }

        c = *fmt++;

        switch(c) {
        case 'd': {
            int v = va_arg(args, int);
            putdec(v);
        }
            break;
        case 'u': {
            unsigned v = va_arg(args, unsigned);
            putudec(v);
        }
            break;
        case 'x': {
            unsigned v = va_arg(args, unsigned);
            puthex(v);
        }
            break;
        case 'c': {
            char v = va_arg(args, int);
            putchar(v);
        }
            break;
        case 'p': {
            uint32_t v = (uint32_t)va_arg(args, void*);
            out8(A9_UART_BASE_1, '0');
            out8(A9_UART_BASE_1, 'x');
            puthex(v);
        }
            break;
        case 's': {
            char c, *v = va_arg(args, char*);
            while((c=*v++)!='\0')
                putchar(c);
        }
            break;
        default:
            putchar('!');
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
