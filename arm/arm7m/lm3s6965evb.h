#ifndef LM3S6965EVB_H
#define LM3S6965EVB_H

#define DEF_ROM_SIZE 0x40000
#define DEF_RAM_SIZE 0x10000

#define M_SYS_BASE  ((volatile void*)0xe000e000u)
#define M_SYS_ICSR  ((volatile void*)0xe000ed04u)
#define M_SYS_SHCSR  ((volatile void*)0xe000ed24u)

#define LM_UART_BASE_1  ((volatile void*)0x4000c000u)
#define UART_BASE_1 LM_UART_BASE_1

#define M_SYSTICK_CSR  ((volatile void*)0xe000e010u)
#define M_SYSTICK_RVR  ((volatile void*)0xe000e014u)
#define M_SYSTICK_CVR  ((volatile void*)0xe000e018u)
#define M_SYSTICK_CAL  ((volatile void*)0xe000e01cu)

#endif // LM3S6965EVB_H
