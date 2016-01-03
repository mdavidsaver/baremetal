#include <stdarg.h>
#include <stdint.h>

#ifdef __linux__
#include <string.h>
#endif

#include "common.h"
#include "termout.h"

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

static
int putint(termdef *t, unsigned neg, char pad, unsigned prec, uint64_t val, unsigned base)
{
    /* a 64-bit unsigned integer has at most 16 base 16 digits, 20 base 10 digits, and 24 base 8 digits */
    char *S, *E;
    int ret;

    if(t->len-t->pos < 25) {
        ret = term_flush(t,0);
        if(ret)
            return ret;
        if(t->len-t->pos < 25)
            return -1;
    }

    S = t->buf + t->pos;

    if(!prec) {
        unsigned n = 0;
        uint64_t v = val;
        for(; v ; v=v/base, n++) {}
        if(n==0)
            n=1;
        prec = n;
    }
    if(prec>24)
        prec = 24; /* paranoia */
    if(pad=='\0')
        pad = ' ';

    t->pos += prec + (neg ? 1 : 0);
    E = t->buf + t->pos;

    if(val) {
        while(val && prec && S<=E) {
            unsigned div = val/base;
            unsigned mod = val%base;
            *--E = hexchars[mod];
            val = div;
            prec--;
        }
    } else {
        *--E = '0';
        prec--;
    }
    while(pad && prec && S<=E) {
        *--E = pad;
        prec--;
    }
    if(neg && S<=E)
        *--E = '-';
    while(S<E)
        *--E = '!'; /* paranoia, this shouldn't happen unless there is a bug above */
    return 0;
}

static
int putdbl(termdef *t, char pad, unsigned prec, unsigned frac, double v)
{
    if(isnan(v)) {
        char buf[] = "nan";
        return term_puts(t, buf);
    } else if(isinf(v)) {
        char buf[] = "-inf";
        return term_puts(t, &buf[v<0 ? 0 : 1]);
    } else {
        uint64_t I;
        unsigned neg = 0;

        if(v<0.0) {
            neg = 1;
            v *= -1.0;
        }

        I = v;
        int ret = putint(t, neg, pad, prec, I, 10);
        if(ret<0)
            return ret;

        v -= I;

        ret = term_putc(t, '.');
        if(ret<0)
            return ret;

        I = v*1000000;
        if(frac==0 || frac>6)
            frac=6;

        while(I && I%10==0)
            I/=10;

        return putint(t, 0, '\0', 0, I, 10);
    }
}

int term_vprintf(termdef *t, const char *fmt, va_list args)
{
    char c;

    while( (c=*fmt++)!='\0')
    {
        int ret;
        if(t->pos==t->len) {
            ret = term_flush(t, 0);
            if(ret)
                return ret;
        }
        if(c!='%') {
            t->buf[t->pos++] = c;
            continue;
        }
        int done = 0;
        unsigned prec = 0, frac = 0, infrac = 0;
        char pad = '\0';

        ret = 0;
readfmt:
        c = *fmt++;

        switch(c) {
        case '.':
            infrac = 1;
            break;
        case ' ':
            pad = ' ';
            break;
        case '0':
            if(pad=='\0' && !infrac) {
                pad = c;
                break;
            }
        case '1' ... '9':
            if(!infrac) {
                prec *= 10;
                prec += c-'0';
            } else {
                frac *= 10;
                frac += c-'0';
            }
            break;
        case 'd': {
            int v = va_arg(args, int);
            unsigned neg = 0;
            if(v<0) {
                neg = 1;
                v = -v;
            }
            ret = putint(t, neg, pad, prec, v, 10);
            done = 1;
        }
            break;
        case 'u': {
            unsigned v = va_arg(args, unsigned);
            ret = putint(t, 0, pad, prec, v, 10);
            done = 1;
        }
            break;
        case 'x': {
            unsigned v = va_arg(args, unsigned);
            ret = putint(t, 0, pad, prec, v, 16);
            done = 1;
        }
            break;
        case 'c': {
            char v = va_arg(args, int);
            ret = term_putc(t, v);
            done = 1;
        }
            break;
        case 'f':
        case 'g': {
            double v = va_arg(args, double);
            ret = putdbl(t, pad, prec, frac, v);
            done = 1;
            break;
        }
        case 'p': {
            size_t v = (size_t)va_arg(args, void*);
            ret = term_puts(t, "0x");
            if(ret==0)
                ret = putint(t, 0, pad, 8, v, 16);
        }
            done = 1;
            break;
        case 's': {
            char *v = va_arg(args, char*);
            ret = term_puts(t, v);
        }
            done = 1;
            break;
        default:
            ret = term_putc(t, '!');
            done = 1;
        }
        if(ret)
            return ret;
        if(!done)
            goto readfmt;
    }
    return 0;
}

int term_printf(termdef *t, const char *fmt, ...)
{
    int ret;
    va_list args; /* I don't have to figure out varargs, gcc to the rescue :) */

    va_start(args, fmt);
    ret = term_vprintf(t, fmt, args);
    va_end(args);
    return ret;
}

int term_putc(termdef *t, char c)
{
    if(t->pos < t->len) {
        t->buf[t->pos++] = c;
    } else {
        int ret = term_flush(t, 0);
        if(ret<0)
            return ret;
    }
    return 0;
}

int term_puts(termdef *t, const char *s)
{
    char c;
    while((c=*s++)!='\0') {
        int ret = term_putc(t, c);
        if(ret)
            return ret;
    }
    return 0;
}

int term_flush(termdef *t, unsigned hw)
{
    if(t->pos>0) {
        // ->out() will never return 0
        int ret = t->out(t, 0);
        if(ret<0) {
            return ret;
        } else if((unsigned)ret < t->pos) {
            memmove(t->buf, t->buf+ret, t->pos-ret);
            t->pos -= ret;
        }
    }
    if(hw) {
        return t->flush(t);
    } else {
        return 0;
    }
}
