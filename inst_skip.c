
#include "armv7m.h"

/* Call on exception entry to get the stack
 * pointer containing the exception frame
 */
uint32_t* get_src_stack(uint32_t *sp)
{
	uint32_t ctrl = 0,
			icsr = in32((void*)0xe000ed04);
	__asm__ ("mrs %r0, CONTROL" : "=r"(ctrl) ::);

    /* TODO: this test can't work since CONTROL<1> always reads
     * zero from a handler.
     * Seems like there is no way to tell from handler mode
     * which stack thread mode was using...
     */
	if((ctrl&2) && (icsr&(1<<11)))
		__asm__ ("mrs %r0, PSP" : "=r"(sp) ::);

	return sp;
}

/* adjust the stacked PC to step
 * forward by one instruction.
 */
void inst_skip(uint32_t *sp)
{
	uint32_t pc = sp[6];
	uint16_t inst = *(uint16_t*)pc;

	if((inst>>11)>=0b11101)
		pc += 4;
	else
		pc += 2;
	sp[6] = pc;
}

int get_svc(uint32_t *sp)
{
	uint32_t pc = sp[6]-2;
	uint16_t inst = *(uint16_t*)pc;

	if((inst&0xff00)!=0xdf00)
		return -1;
	
	return inst&0xff;
}
