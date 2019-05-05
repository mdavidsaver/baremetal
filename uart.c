// MPC8540 DUART

#include "common.h"
#include "mmio.h"

uint32_t ccsr_base = 0xe1000000;
uint32_t uart_base = 0xe1000000 + 0x4500;
#define UART uart_base

static
void uart_tx(char c)
{
    while(!(in8x(UART, 0x5)&0x40)) {}
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

void puts(const char *s)
{
    char c;
    while((c=*s++)!='\0')
        putc(c);
}
