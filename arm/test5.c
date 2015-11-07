/* Test Mode/Stack changes
 */
#include "armv7m.h"

extern char _main_stack_top, _proc_stack_top;
extern char _main_stack_bot, _proc_stack_bot;

static
unsigned test;

static
void test_equal(const char *msg, uint32_t lhs, uint32_t rhs)
{
	puts(lhs==rhs ? "ok - " : "fail - ");
	puthex(lhs);
	puts(" == ");
	puthex(rhs);
	puts(" # ");
	puts(msg);
	putc('\n');
}

static
void show_control(unsigned ectrl, unsigned evect)
{
	uint32_t actrl, avect;
	unsigned instack;
	char *sp;
	__asm__ ("mov %0,sp" : "=r"(sp) ::);
	__asm__ ("mrs %0,IPSR" : "=r"(avect) ::);
	__asm__ ("mrs %0,CONTROL" : "=r"(actrl) ::);
	puts(" SP ");
	puthex((uint32_t)sp);
	putc('\n');

	test_equal("CONTROL", ectrl, actrl);
	test_equal("IPSR", evect, avect);

	if(sp>&_main_stack_bot && sp<=&_main_stack_top)
		instack = 0;
	else if(sp>&_proc_stack_bot && sp<=&_proc_stack_top)
		instack = 2;
	else {
		puts("fail - Corrupt SP ");
		puthex((uint32_t)sp);
		putc('\n');
		return;
	}
	if((avect>0 && instack==0) || (avect==0 && (actrl&2)==instack))
		puts("ok - stack ");
	else
		puts("fail - stack ");
	putc(instack?'2':'0');
	putc('\n');
}

static
void show_masks(unsigned expect)
{
	uint32_t val, temp;
	__asm__ volatile ("mrs %0,PRIMASK" : "=r"(val)::);
	__asm__ volatile ("mrs %0,FAULTMASK" : "=r"(temp)::);
	val |= temp<<1;
	__asm__ volatile ("mrs %0,BASEPRI" : "=r"(temp)::);
	val |= temp<<2;
	test_equal("masks", expect, val);
}


static
void svc(void)
{
	switch(test) {
	case 1:
		puts("2. SVC\n");
		show_control(0, 11);
		break;
	case 2:
		puts("6. SVC\n");
		show_control(2, 11);
		break;
	case 3:
		puts("10. SVC\n");
		show_control(3, 11);
		break;
	case 4:
		puts("15. SVC\n");
		show_masks(0);
		__asm__ volatile ("cpsid if" :::);
		show_masks(3);
		break;
	default:
		puts("Fail SVC\n");
		abort();
	}
}

void main(void)
{
	run_table.svc = svc;

	{
		uint32_t temp = (uint32_t)&_proc_stack_top;
		__asm__ volatile ("msr PSP,%0" :: "r"(temp) :);
	}

	puts(" MSP ");
	puthex((uint32_t)&_main_stack_top);
	puts("\n PSP ");
	puthex((uint32_t)&_proc_stack_top);
	putc('\n');

	show_control(0, 0);

	test = 1;
	puts("1. Start, trigger SVC\n");
	__asm__ volatile ("cpsie if" :::);
	__asm__ volatile ("svc 42" :::);

	puts("3. Back in main\n");
	show_control(0, 0);

	puts("4. Priv w/ proc stack\n");
	{
		uint32_t val = 2;
		__asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
	}
	show_control(2, 0);

	test = 2;
	puts("5. trigger SVC\n");
	__asm__ volatile ("svc 42" :::);

	puts("7. Back in main\n");
	show_control(2, 0);

	test = 3;
	puts("8. Drop privlage\n");
	{
		uint32_t val = 3;
		__asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
	}
	show_control(3, 0);

	puts("9. trigger SVC\n");
	__asm__ volatile ("svc 42" :::);

	puts("11. Back in main\n");
	show_control(3, 0);

	puts("12. Try to restore privlage and switch stack (should be noop)\n");
	{
		uint32_t val = 0;
		__asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
	}
	show_control(3, 0);

	test = 4;
	puts("13. Try to set masks\n");
	__asm__ volatile ("cpsid if" :::);
	show_masks(0); /* doesn't work */

	puts("14. trigger SVC\n");
	__asm__ volatile ("svc 42" :::);

	puts("16. Back in main\n");
	show_masks(0); /* unprivlaged doesn't see mask */

	puts("Done.\n");
}
