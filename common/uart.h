#ifndef UART_H
#define UART_H

void uart_putc(char c);
void uart_puts(const char *s);
void uart_flush(void);

#endif // UART_H
