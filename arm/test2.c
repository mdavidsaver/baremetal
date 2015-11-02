/* Test exception escalation in armv7-m
 */
#include "armv7m.h"

static int test;

static __attribute__((unused))
void hard(uint32_t *sp)
{
	uint32_t temp;
	switch(test) {
	case 0:
		__asm__ ("ldr %0, =recover0" : "=r"(temp) ::);
		puts("2. HardFault\n");
		break;
	case 1:
		__asm__ ("ldr %0, =recover1" : "=r"(temp) ::);
		puts("5. HardFault\n");
		break;
	case 3:
		__asm__ ("ldr %0, =recover4" : "=r"(temp) ::);
		puts("12. HardFault\n");
		break;
	default:
		puts("HardFault Fail\n");
		abort();
	}
	sp[6] = temp;
}

static __attribute__((unused))
void usage(uint32_t *sp)
{
	uint32_t temp;
	switch(test) {
	case 2:
		__asm__ ("ldr %0, =recover2" : "=r"(temp) ::);
		puts("8. UsageFault\n");
		break;
	case 3:
		puts("11. UsageFault\n");
		__asm__ ("ldr %0, =recover3" : "=r"(temp) ::);
		__asm__ (".word 0xffff; recover4:" :::);
		puts("13. Back in usage().\n");
		break;
	default:
		puts("Fail\n");
		abort();
	}
	sp[6] = temp;
}

static __attribute__((naked))
void hardx(void)
{
	asm ("mov r0, sp"); // save stack pointer on entry
	asm ("b hard");
}

static __attribute__((naked))
void usagex(void)
{
	asm ("mov r0, sp"); // save stack pointer on entry
	asm ("b usage");
}

void main(void)
{
	run_table.hard = hardx;
	run_table.usage = usagex;
	puts("1. Fault to HardFault\n");
	/* this would trigger a usage error, except that
	 * UsageFault isn't enabled and
	 * FAULTMASK and PRIMASK are set
	 */
	__asm__ (".word 0xffff; recover0:" :::);
	puts("3. Back in main\n");

	out32((void*)0xe000ed24, 1<<18); // enable UsageFault with SHCSR

	test = 1;
	puts("4. Fault to HardFault\n");
	__asm__ ("cpsid f" :::); /* FAULTMASK auto-cleared when HardFault returns */
	/* this would trigger a usage error, except that
	 * FAULTMASK and PRIMASK are set
	 */
	__asm__ (".word 0xffff; recover1:" :::);
	puts("6. Back in main\n");
	
	test = 2;
	__asm__ ("cpsie if" :::);
	puts("7. Faulting to UsageFault\n");
	/* This time really trigger UsageFault
	 */
	__asm__ (".word 0xffff; recover2:" :::);
	puts("9. Back in main\n");
	
	test = 3;
	puts("10. Faulting to UsageFault then HardFault\n");
	/* This time trigger UsageFault,
	 * then fault in UsageFault into HardFault
	 */
	__asm__ (".word 0xffff; recover3:" :::);
	puts("14. Back in main 4\n");

	puts("Done\n");
}
