
#include "cpu.h"
#include "common.h"

void show_arm_id(void)
{
    uint32_t cpuid=0;
#ifdef ARM7M
    cpuid = in32((void*)0xe000ed00);
#else
    asm volatile ("mrc p15, 0, %0, c0, c0, 0" : : "r"(cpuid) : "memory" );
#endif
    printk("CPU ID:\n"
              " Implimenter: %x\n", (unsigned)EXTRACT(cpuid, 24, 31));
    printk(" Variant:     %x\n", (unsigned)EXTRACT(cpuid, 20, 23));
    printk(" Part#:       %x\n", (unsigned)EXTRACT(cpuid, 4, 15));
    printk(" Revision#:   %x\n", (unsigned)EXTRACT(cpuid, 0, 3));
}
