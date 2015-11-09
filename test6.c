/* Test unrecoverable exception
 */
#include "armv7m.h"

static
void hard(void)
{
	puts("In HardFault\noops...\n");
	abort();
}

void main(void)
{
	run_table.hard = &hard;
	puts("Faulting...\n");
	/* Trigger a UsageFault, which will escalate to unrecoverable
	 * since FAULTMASK is set on start
	 */
	__asm__ volatile (".word 0xffff");
	puts("Oops, I shouldn't be here...\n");
}
