/* Test exception handling registers
 */
#include "armv7m.h"

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
volatile unsigned test;

static
void pendsv(void)
{
    switch(test) {
    case 2:
        puts("3. In PendSV\n");
        /* RETTOBASE not set */
        test_equal("ICSR ", 0x0000000e, in32(SCB(0xd04)));
        test_equal("SHCSR", 0x00000480, in32(SCB(0xd24)));
        test = 3;
        break;
    default:
        puts("Unexpected PendSV\n");
        abort();
    }
}

static
void svc(void)
{
    switch(test) {
    case 1:
        puts("2. In SVC\n");
        test = 2;
        /* RETTOBASE set */
        test_equal("ICSR ", 0x0000080b, in32(SCB(0xd04)));
        test_equal("SHCSR", 0x00000080, in32(SCB(0xd24)));
        rmw(32, SCB(0xd04), 1<<28, 1<<28); /* Pend PendSV */
        puts("4. Back in SVC\n");
        test_equal("test ", 3, test);
        test = 4;
        break;
    default:
        puts("Unexpected SVC\n");
        abort();
    }
}

void main(void)
{
    run_table.svc = &svc;
    run_table.pendsv = &pendsv;

    rmw(32,SCB(0xd0c),0x700, PRIGROUP<<8);
    out32(SCB(0xd1c), PRIO(2,0)<<24); /* SVC prio 2 */
    out32(SCB(0xd20), PRIO(1,0)<<16); /* PendSV prio 1 */

    test = 1;
    /* RETTOBASE not set */
    test_equal("ICSR ", 0x00000000, in32(SCB(0xd04)));
    test_equal("SHCSR", 0x00000000, in32(SCB(0xd24)));
    puts("1. Call SVC\n");
    SVC(42);
    puts("5. Back in main\n");
    test_equal("test ", 4, test);
    test = 5;
    test_equal("ICSR ", 0x00000000, in32(SCB(0xd04)));
    test_equal("SHCSR", 0x00000000, in32(SCB(0xd24)));

    puts("Done\n");
}
