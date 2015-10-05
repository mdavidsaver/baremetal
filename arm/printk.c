#include <stdarg.h>

#include "common.h"

/* IEEE 754 (1985) floating point format (double precision)
 * | sign (1 bit) | exp (11 bits) | fraction (52 bits) |
 *
 * exp_offset = (1<<10)-1                    // 1023
 * frac_base = 1<<52                         // 0x0800000
 * frac_nrom = 1<<53                         // 0x1000000
 * val = (frac_nrom+frac)/frac_base*(1<<(exp-exp_offset)
 */

#define DBL_SIGN_MASK 0x8000000000000000ull
#define DBL_SIGN_SHIFT 63
#define DBL_EXP_MASK  0x7ff0000000000000ull
#define DBL_EXP_SHIFT  52
#define DBL_FRAC_MASK 0x000fffffffffffffull
#define DBL_FRAC_SHIFT 0

#define DBL_EXP_OFFSET 1023
#define DBL_EXP_MAX    0x7ff

#define DBL_SIGN(I) ((I)&DBL_SIGN_MASK)
#define DBL_EXP_U(I) (((I)&DBL_EXP_MASK)>>DBL_EXP_SHIFT)
#define DBL_EXP(I) (DBL_EXP_OFFSET-(int)DBL_EXP_U(I))
#define DBL_FRAC(I) (((I)&DBL_FRAC_MASK)>>DBL_FRAC_SHIFT)

union pun_double {
    double val;
    uint64_t I;
};

static
int isnan(double I)
{
    union pun_double V;
    V.val = I;
    return DBL_EXP_U(V.I)==DBL_EXP_MAX && DBL_FRAC(V.I)!=0;
}

static
int isinf(double I)
{
    union pun_double V;
    V.val = I;
    return DBL_EXP_U(V.I)==DBL_EXP_MAX && DBL_FRAC(V.I)==0;
}

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

void puts(const char *str)
{
    char c;
    while( (c=*str++)!='\0' )
        out8(A9_UART_BASE_1, c);
}

static
void putdbl(double v)
{
    if(isnan(v)) {
        puts("nan");
    } else if(isinf(v)) {
        if(v<0.0) putchar('-');
        puts("inf");
    } else {
        uint64_t I;
        unsigned N = __DBL_DIG__;
        if(v<0.0) {
            putchar('-');
            v *= -1.0;
        }
        I = v;
        putudec(I); /* TODO, real 64-bit */
        v -= I;
        putchar('.');
        while(N--) {
            v *= 10.0;
            I = v;
            putudec(I);
            v -= I;
        }
    }
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
        case 'f':
        case 'g': {
            double v = va_arg(args, double);
            if(isnan(v)) {
                puts("nan");
            } else if(isinf(v)) {
                if(v<0.0) putchar('-');
                puts("inf");
            } else {
                putdbl(v);
            }

            break;
        }
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
