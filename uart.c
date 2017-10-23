// MPC8540 DUART

#include "common.h"
#include "mmio.h"

uint32_t ccsr_base = 0xe1000000;
#define UART (ccsr_base+0x4500)

static
const char hexchars[] = "0123456789ABCDEF";

static
void uart_tx(char c)
{
    //TODO: proper wait for TX not full
    //while(!(in8x(UART, 0x5)&(1<<1))) {}
    out8x(UART, 0x0, c);
}

void putc_escape(char c)
{
    switch(c) {
    case '\t':
        uart_tx('\\');
        uart_tx('t');
        break;
    case '\n':
        uart_tx('\\');
        uart_tx('n');
        break;
    case '\r':
        uart_tx('\\');
        uart_tx('r');
        break;
    case ' ' ... '~':
        uart_tx(c);
        break;
    default:
        uart_tx('\\');
        uart_tx('x');
        uart_tx(hexchars[c>>4]);
        uart_tx(hexchars[c&0xf]);
    }
}

void putc(char c)
{
    if(c=='\n') uart_tx('\r');
    uart_tx(c);
}

static
void puts(const char *s)
{
    char c;
    while((c=*s++)!='\0')
        putc(c);
}

static
void puthex(uint32_t val)
{
    unsigned i;
    for(i=0; i<8; i++, val<<=4)
        uart_tx(hexchars[val>>28]);
}

void vprintk(const char *fmt, va_list args)
{
    char c;
    while((c=*fmt++)!='\0') {
        switch(c) {
        case '\n':
            uart_tx('\r');
        default:
            uart_tx(c);
            break;
        case '%':
            c=*fmt++;
            switch(c) {
            case 's': {
                const char * v = va_arg(args, const char*);
                puts(v);
                break;
            }
            case 'x': {
                unsigned v = va_arg(args, unsigned);
                puthex(v);
                break;
            }
            case '\0':
                uart_tx('!');
                return;
            default:
                uart_tx('!');
            }
        }
    }
}

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
