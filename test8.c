/* Test MPU
 */
#include "armv7m.h"

char __end_rom, __after_all_load;

char offlimits[1024] __attribute__((aligned(1024)));

char privonly[1024] __attribute__((aligned(1024)));

static
void try(volatile char *p)
{
	uint32_t tval = 0;
	puts("Try ");
	puthex((uint32_t)p);
	putc('\n');
	__asm__ ("mov %r0, #'X'; ldr %r0, [%r1]" : "+r"(tval) : "r"(p) :);
	puts("Got ");
	putc(tval);
	putc('\n');
}

static volatile
unsigned expect_fault = 1;

static
void checkfault(unsigned v)
{
	if(v!=expect_fault) {
		puts("expect_fault ");
		puthex(expect_fault);
		puts(" != ");
		puthex(v);
		putc('\n');
		abort();
	}
}

void hard(uint32_t *sp)
{
	if(expect_fault==10) {
		sp = get_src_stack(sp);
		inst_skip(sp);
		puts("HardFault access offlimits\n");
		try(offlimits);
		expect_fault = 11;
		return;
	}
	puts("Unexpected HardFault!!\n");
	abort();
}

static __attribute__((naked))
void hard_entry(void)
{
	asm("mov r0, sp");
	asm("b hard");
}

void mem(uint32_t *sp)
{
	sp = get_src_stack(sp);
	inst_skip(sp);
	puts("In MemFault\n Addr ");
	puthex(in32((void*)0xe000ed34));
	puts(" From ");
	puthex(sp[6]);
	putc('\n');

	switch(expect_fault) {
	case 2: /* expected */
		expect_fault = 3;
		break;
	case 1:
		puts("Unexpected MemFault!!\n");
		abort();
	default:
		puts("Unexpected state ");
		puthex(expect_fault);
		putc('\n');
		abort();
	}
}

static __attribute__((naked))
void mem_entry(void)
{
	asm("mov r0, sp");
	asm("b mem");
}

static inline
void drop_priv(void)
{
	uint32_t val;
	__asm__ ("mrs %0, CONTROL" : "=r"(val) ::);
	val |= 1;
	__asm__ ("msr CONTROL, %0" :: "r"(val) :);
}

void svc(void *sp)
{
	int num;
	sp = get_src_stack(sp);
	num = get_svc(sp);
	puts("In SVC ");
	if(num<0) {
		puts("BAD\n");
		abort();
	} else {
		puthex(num);
		putc('\n');
	}
	switch(num) {
	case 1: /* restore privlage */
	{
		uint32_t val;
		__asm__ ("mrs %0, CONTROL" : "=r"(val) ::);
		val &= ~1;
		__asm__ ("msr CONTROL, %0" :: "r"(val) :);
	}
		break;
	case 2:
		abort();
	default:
		puts("Unknown\n");
		abort();
	}
}

static __attribute__((naked))
void svc_entry(void)
{
	asm("mov r0, sp");
	asm("b svc");
}

void main(void)
{
	run_table.hard = &hard_entry;
	run_table.svc = &svc_entry;
	run_table.mem = &mem_entry;

	asm("cpsid if");
    out32(SCB(0xd24), 1<<16); // enable MemFault with SHCSR

	puts("1. In Main\n");
	expect_fault = 1;

	/* priority of entries is highest to lowest (0 checked last) */

	/* base for ROM */
	set_mpu(0, 0, (uint32_t)&__end_rom,
			MPU_NORMAL|MPU_RORO);
	/* prevent accidental *NULL */
//	set_mpu(1, 0, 64,
//			MPU_XN|MPU_NORMAL|MPU_NANA);
	/* base for RAM */
	set_mpu(2, 0x20000000, (uint32_t)(&__after_all_load)-0x20000000,
			MPU_NORMAL|MPU_RWRW);
	/* allow unpriv uart access */
	set_mpu(3, (uint32_t)UART_DATA, 1024,
			MPU_XN|MPU_DEVICE|MPU_RWRW);

	/* disable all access to offlimits[] */
	set_mpu(4, (uint32_t)offlimits, sizeof(offlimits),
			MPU_XN|MPU_NORMAL|MPU_NANA);
	/* disable unpriv access to privonly[] */
	set_mpu(5, (uint32_t)privonly, sizeof(privonly),
			MPU_XN|MPU_NORMAL|MPU_RONA);

	puts("2. Access offlimits\n");
	try(offlimits);
	checkfault(1);

	puts("3. Enable MPU\n");
	enable_mpu(1,0);

	asm("cpsie if");

	expect_fault = 2;
	puts("4. Access offlimits\n");
	try(offlimits);
	checkfault(3);

	puts("6. In Main\n");
	expect_fault = 1;

	puts("7. Access privonly\n");
	try(privonly);
	checkfault(1);

	puts("8. drop_priv\n");
	expect_fault = 2;
	drop_priv();
	puts("9. Access privonly\n");
	try(privonly);

	puts("11. In Main\n");
	asm("svc 1");
	checkfault(3);
	expect_fault = 1;

	puts("12. Access privonly\n");
	try(privonly);
	checkfault(1);

	puts("13. In Main\n");
	asm("cpsid i");


	puts("14. fault to hardfault\n");
	expect_fault = 10;
	try(offlimits);
	checkfault(11);

	puts("15. Enable MPU w/ HFNMIENA\n");
	enable_mpu(1,1);
	puts("16. fault to hardfault (will escalate to unrecoverable)\n");
	expect_fault = 10;
	try(offlimits);

	puts("Shouldn't be here!\n");
}
