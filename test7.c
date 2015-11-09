/* Test exception return from thread mode
 */
#include "armv7m.h"

static
void hard(void)
{
	puts("In HardFault\n");
	abort();
}

static
void func(void)
{
	uint32_t temp = 0xfffffff9; /* return to thread mode, main stack */
	__asm__ volatile ("mov lr,%0" ::"r"(temp):);
	return;
}

void main(void)
{
	run_table.hard = &hard;
	puts("Starting\n");
	func();
	puts("oops, I shouldn't be here\n");
}
