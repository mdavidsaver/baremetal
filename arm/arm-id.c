
#include "common.h"

void show_arm_id(void)
{
    uint32_t cpuid=0;
#ifdef __ARM_ARCH_7M__
    cpuid = in32((void*)0xe000ed00);
#else
    asm volatile ("mrc p15, 0, %0, c0, c0, 0" : : "r"(cpuid) : "memory" );
#endif
    printk(0, "CPU ID:\n"
              " Implimenter: %x\n", (unsigned)EXTRACT(cpuid, 24, 31));
    printk(0, " Variant:     %x\n", (unsigned)EXTRACT(cpuid, 20, 23));
    printk(0, " Part#:       %x\n", (unsigned)EXTRACT(cpuid, 4, 15));
    printk(0, " Revision#:   %x\n", (unsigned)EXTRACT(cpuid, 0, 3));
}
