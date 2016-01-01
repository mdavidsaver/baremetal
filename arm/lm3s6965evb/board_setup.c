
#include "bsp.h"
#include "arm7m.h"

void uart_putc(char c)
{
    loop_until(32, UART_FLAG, 0x20, ==, 0); /* wait for TX FIFO not full */
    out8(UART_DATA, c);
}

void uart_puts(const char *s)
{
    char c;
    while((c=*s++)!='\0')
        uart_putc(c);
}

void uart_flush(void)
{
    loop_until(32, UART_FLAG, 0x80, ==, 0x80); /* wait for TX FIFO empty */
}
