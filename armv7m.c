
#include "armv7m.h"


void putc(char c)
{
    if(c=='\n')
        putc('\r');
    loop_until(32, UART_FLAG, 0x20, ==, 0); /* wait for TX FIFO not full */
    out8(UART_DATA, c);
}

void puts(const char *s)
{
    char c;
    while((c=*s++)!='\0')
        putc(c);
}

void flush(void)
{
    /* wait for TX FIFO empty */
    loop_until(32, UART_FLAG, 0x80, ==, 0x80);
}


void puthex(uint32_t v)
{
    unsigned i;
    for(i=0; i<8; i++, v<<=4) {
        putc(hexchars[v>>28]);
    }
}

void putdec(uint32_t v)
{
    unsigned out=0;
    uint32_t base=1000000000;
    for(;base; base/=10) {
        uint32_t d=v/base;
        if(!d && !out) continue;
        putc('0'+d);
        v%=base;
        out=1;
    }
    if(!out) putc('0');
}

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}

void vprintk(const char *fmt, va_list args)
{
    while(1) {
        char c=*fmt++;
        if(c=='\0') {
            return;

        } else if(c!='%') {
            putc(c);

        } else {
specmod:
            c = *fmt++;
            
            switch(c) {
            case '\0': return;
            case '%': putc(c); break;
            case '0'...'9':
            case ' ':
                /* ignore modifiers */
                goto specmod;
            case 'u': {
                unsigned v = va_arg(args, unsigned);
                putdec(v);
                break;
            }
            case 'c': {
                char v = va_arg(args, int);
                putc(v);
                break;
            }
            case 'x': {
                unsigned v = va_arg(args, unsigned);
                puthex(v);
                break;
            }
            case 'p': {
                const void *v = va_arg(args, const void*);
                puthex((uint32_t)v);
                break;
            }
            case 's': {
                const char *v = va_arg(args, const char*);
                if(v)
                    puts(v);
                else
                    puts("<null>");
                break;
            }
            default:
                putc('!');
                break;
            }
        }
    }
}

unsigned log2_ceil(uint32_t v)
{
    unsigned r=0, c=0;
    while(v) {
        c += v&1;
        v >>= 1;
        r++;
    }
    if(c>1) r++;
    return r;
}

void set_mpu(unsigned region, uint32_t base, uint32_t size,
             uint32_t attrs)
{
    unsigned sbits = log2_ceil(size<32 ? 32 : size)-2;
    uint32_t rbase = base&(~0x1f);
    uint32_t rattr = (attrs&~0xffff) | (sbits<<1) | 1;
    puts("set_mpu ");
    putc('0'+region);
    putc(' ');
    puthex(rbase);
    putc(' ');
    puthex(rattr);
    putc('\n');
    out32((void*)0xe000ed98, region&0xff); /* RNR */
    out32((void*)0xe000eda0, 0); /* Disable */
    out32((void*)0xe000ed9c, rbase);
    out32((void*)0xe000eda0, rattr);
}

void enable_mpu(unsigned usrena, unsigned privena, unsigned hfnmiena)
{
    uint32_t val = (usrena ? 1 : 0) | (hfnmiena ? 2 : 0) | (privena ? 0 : 4);
    out32((void*)0xe000ed94, val);
    __asm__ volatile ("dsb\nisb" :::);
}
