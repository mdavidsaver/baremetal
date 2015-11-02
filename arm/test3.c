/* Test exception escalation in armv7-m
 */
#include "armv7m.h"

static int test;

static
void hard(void)
{
	switch(test) {
	case 2:
		puts("8. in HardFault\n");
		break;
	case 3:
		puts("11. in HardFault\n");
		break;
	case 5:
		puts("17. in HardFault\n");
		break;
	default:
		puts("Fail HardFault\n");
		abort();
	}
}

static
void svc(void)
{
	switch(test) {
	case 0:
		puts("2. in SVC\n");
		break;
	case 4:
		puts("14. in SVC\n");
		break;
	default:
		puts("Fail SVC\n");
		abort();
	}
}

static __attribute__((unused))
void pendsv(void)
{
	switch(test) {
	case 1:
		puts("5. in PendSV\n");
		break;
	default:
		puts("Fail PendSV\n");
		abort();
	}
}

void main(void)
{
	uint32_t temp;
	run_table.hard = hard;
	run_table.pendsv = pendsv;
	run_table.svc = svc;

	test = 0;
	__asm__ ("cpsie if" :::);
	puts("1. trigger SVC\n");
	__asm__ ("svc 42" :::);
	puts("3. Back in main\n");
	
	test = 1;
	puts("4. trigger PendSV\n");
	out32((void*)0xe000ed04, 1<<28);
	puts("6. Back in main\n");

	test = 2;
	puts("7. trigger HardFault\n");
	__asm__ ("cpsid i" :::); /* mask prio<=0 */
	__asm__ ("svc 42" :::);
	puts("9. Back in main\n");

	test = 3;
	__asm__ ("cpsie i" :::);
	temp = 1;
	__asm__ ("msr BASEPRI, %0" : : "r"(temp) :);

	puts("10. trigger HardFault\n");
	__asm__ ("svc 42" :::);
	puts("12. Back in main\n");

	test = 4;
	temp = 2;
	__asm__ ("msr BASEPRI, %0" : : "r"(temp) :); /* mask prio<=1 */
	puts("13. trigger SVC\n");
	__asm__ ("svc 42" :::);
	puts("15. Back in main\n");

	test = 5;
	/* PRIGROUP==0, so 0x02 sets group priority 1 */
	out32((void*)0xe000ed1c, 0x02000000);
	puts("16. trigger HardFault\n");
	__asm__ ("svc 42" :::);
	puts("18. Back in main\n");

	puts("Done\n");
}
