#ifndef LM3S6965EVB_H
#define LM3S6965EVB_H

#define DEF_ROM_SIZE 0x40000
#define DEF_RAM_SIZE 0x10000

#define M_SYS_BASE  ((volatile void*)0xe000e000u)

#define LM_UART_BASE_1  ((volatile void*)0x4000c000u)
#define UART_BASE_1 LM_UART_BASE_1

#endif // LM3S6965EVB_H
