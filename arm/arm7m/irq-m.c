
#include "common.h"

void irq_setup(void)
{
    asm volatile ("dmb\n cpsie i" ::: "memory");
}

void irq_show(void)
{

}
