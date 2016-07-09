/* Test exception handling registers
 */
#include "armv7m.h"
#include "testme.h"

static
unsigned testseq;

#define SEQ() __atomic_add_fetch(&testseq, 1, __ATOMIC_RELAXED)

static
void pendsv(void)
{
    unsigned test = SEQ();
    switch(test) {
    case 2:
        testDiag("In PendSV");
        /* RETTOBASE not set */
        testEqI(0x0000000e, in32(SCB(0xd04)), "ICSR");
        testEqI(0x00000480, in32(SCB(0xd24)), "SHCSR");
        break;
    default:
        puts("Unexpected PendSV\n");
        abort();
    }
}

static
void svc(void)
{
    unsigned test = SEQ();
    switch(test) {
    case 1:
        testDiag("In SVC");
        /* RETTOBASE set */
        testEqI(0x0000080b, in32(SCB(0xd04)), "ICSR");
        testEqI(0x00000080, in32(SCB(0xd24)), "SHCSR");
        rmw(32, SCB(0xd04), 1<<28, 1<<28); /* Pend PendSV */
        testDiag("Back in SVC");
        test = SEQ();
        testEqI(3, test, "Back in SVC");
        break;
    default:
        testFail("Unexpected SVC");
        abort();
    }
}

void main(void)
{
    run_table.svc = &svc;
    run_table.pendsv = &pendsv;
    
    testInit(10);

    {
        /* attempt to detect the number of usable
         * bits in the priority fields.
         */
        uint32_t val = 0xff;
        __asm__ ("msr BASEPRI, %0" :: "r"(val) :);
        __asm__ ("mrs %0, BASEPRI" : "=r"(val) ::);
        puts("# BASEPRI mask ");
        puthex(val);
        val = 0;
        __asm__ ("msr BASEPRI, %0" :: "r"(val) :);
        rmw(32, SCB(0xd20), 0xff, 0xff);
        val = in32(SCB(0xd20))&0xff;
        rmw(32, SCB(0xd20), 0xff, 0);
        puts("\n# DEBUG prio ");
        puthex(val);
        putc('\n');
    }

    out32(SCB(0xd0c), 0x05fa0000 | (PRIGROUP<<8));
    out32(SCB(0xd1c), PRIO(2,0)<<24); /* SVC prio 2 */
    out32(SCB(0xd20), PRIO(1,0)<<16); /* PendSV prio 1 */

    /* RETTOBASE not set */
    testEqI(0x00000000, in32(SCB(0xd04)), "ICSR");
    testEqI(0x00000000, in32(SCB(0xd24)), "SHCSR");
    testDiag("Call SVC");
    SVC(42);
    testDiag("Back in main");
    testEqI(4, SEQ(), "Back in SVC");
    testEqI(0x00000000, in32(SCB(0xd04)), "ICSR");
    testEqI(0x00000000, in32(SCB(0xd24)), "SHCSR");

    testDiag("Done");
}
