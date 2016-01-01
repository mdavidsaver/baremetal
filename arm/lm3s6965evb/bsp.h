#ifndef LM3S6965EVB_H
#define LM3S6965EVB_H

#include "cpu.h"

#define DEF_ROM_SIZE 0x40000
#define DEF_RAM_SIZE 0x10000

#define M_SYS_ICSR  ((volatile void*)0xe000ed04u)
#define M_SYS_SHCSR  ((volatile void*)0xe000ed24u)

#define UART(N) ((N)+(void*)0x4000c000)
#define UART_DATA UART(0)
#define UART_FLAG UART(0x18)

#endif // LM3S6965EVB_H
