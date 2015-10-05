#ifndef VEXPRESSA9_H
#define VEXPRESSA9_H

/* register bases */
#define A9_SYSCTRL_BASE ((volatile void*)0x10000000u)

#define A9_UART_BASE_1  ((volatile void*)0x10009000u)
#define UART_BASE_1 A9_UART_BASE_1

/* interrupt controller */
#define A9_PIC_CPU_SELF ((volatile void*)0x1e000100u)
#define A9_PIC_CONF     ((volatile void*)0x1e001000u)

/* ARM sp804 */
#define A9_TIMER_BASE_1 ((volatile void*)0x10011000u)

#endif // VEXPRESSA9_H
