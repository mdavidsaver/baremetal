/* Test IRQ masking and priorities in armv7-m
 */

// This test needs at least 1 bit sub-priority on TM4C
#define PRIGROUP 5

#include "armv7m.h"
#include "testme.h"

static
unsigned testseq;

#define SEQ() __atomic_add_fetch(&testseq, 1, __ATOMIC_RELAXED)

#define CHECK_SEQ(N) testEqI(N, SEQ(), "SEQ " #N)

static inline
void test_equal(const char *m, uint32_t expect, uint32_t actual)
{
    testEqI(expect, actual, m);
}

static
void irq0(void)
{
    unsigned test = SEQ();
    printk("# in IRQ0 SEQ %u\n", test);
    switch(test) {
    case 2:
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 0, in32(SCB(0x200)));
        test_equal("ACT ", 1, in32(SCB(0x300)));
        test_equal("ICSR", 0x00000810, in32(SCB(0xd04)));
        break;
    case 5:
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 0, in32(SCB(0x200)));
        test_equal("ACT ", 1, in32(SCB(0x300)));
        test_equal("ICSR", 0x00000810, in32(SCB(0xd04)));
        break;
    case 7:
        testDiag("pend IRQ1");
        __asm__ ("dsb" ::: "memory");
        out32(SCB(0x200), 2);
        testDiag("back in IRQ0");
        CHECK_SEQ(9);
        break;
    case 13:
        break;
    case 16:
        break;
    case 18:
        __asm__ ("dsb" ::: "memory");
        break;
    default:
        testFail("Fail IRQ0");
        abort();
    }
}

static
void irq1(void)
{
    unsigned test = SEQ();
    printk("# in IRQ1 SEQ %u\n", test);
    switch(test) {
    case 4:
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 1, in32(SCB(0x200)));
        test_equal("ACT ", 2, in32(SCB(0x300)));
        /* ISRPENDING, VECTPENDING==16, RETTOBASE, and VECACTIVE=17 */
        test_equal("ICSR", 0x00410811, in32(SCB(0xd04)));
        break;
    case 8:
        break;
    case 11:
        testDiag("pend IRQ0");
        out32(SCB(0x200), 1);
        __asm__ ("dsb" ::: "memory");
        testDiag("still in IRQ1");
        CHECK_SEQ(12);
        __asm__ ("dsb" ::: "memory");
        break;
    case 15:
        break;
    case 19:
        break;
    default:
        testFail("Fail IRQ1");
        abort();
    }
}

void main(void)
{
    run_table.irq[0] = irq0;
    run_table.irq[1] = irq1;
    
    testInit(33);

    out32(SCB(0xd0c), 0x05fa0000 | (PRIGROUP<<8));
    test_equal("PRIGROUP", PRIGROUP, (in32(SCB(0xd0c))>>8)&0xff);

    CPSID(i);

    testDiag("Enable IRQ0/1");
    out32(SCB(0x100), 3);

    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 0, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));
    test_equal("ICSR", 0x00000000, in32(SCB(0xd04)));

    testDiag("Pend IRQ0 (shouldn't run)");
    out32(SCB(0x200), 1);
    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 1, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));
    /* ISRPENDING and VECTPENDING==16 */
    test_equal("ICSR", 0x00410000, in32(SCB(0xd04)));

    CHECK_SEQ(1);
    testDiag("Unmask (should run now)");
    CPSIE(i);

    testDiag("Back in main");
    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 0, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));

    CHECK_SEQ(3);
    testDiag("Give IRQ1 priority over IRQ0");
    out32(SCB(0x400), (PRIO(1,0)<<8)|PRIO(2,0));

    testDiag("Pend IRQ0 and IRQ1 (should run now)");
    out32(SCB(0x200), 3);

    testDiag("Back in main");

    /* see that one handler can really pre-empt another */
    CHECK_SEQ(6);
    testDiag("Pend IRQ0 (should run now)");
    out32(SCB(0x200), 1);

    testDiag("Back in main");

    /* see that one handler doesn't pre-empt another
     * when it has lower priority
     */
    CHECK_SEQ(10);
    testDiag("Pend IRQ1 (should run now)");
    out32(SCB(0x200), 2);

    testDiag("Back in main");

    /* check sub-group ordering */
    CHECK_SEQ(14);
    testDiag("equal prio, IRQ1 has lower sub-group");
    out32(SCB(0x400), (PRIO(0,0)<<8)|PRIO(0,0x3f));
    testDiag("Pend IRQ0 and IRQ1 (should run now)");
    out32(SCB(0x200), 3);

    testDiag("Back in main");

    /* check fallback it vector # */
    CHECK_SEQ(17);
    out32(SCB(0x400), (PRIO(0,0)<<8)|PRIO(0,0)); /* equal prio, and sub-group, IRQ0 has lower vector */
    testDiag("Pend IRQ0 (should run now) and IRQ1");
    out32(SCB(0x200), 3);

    testDiag("Back in main");
    CHECK_SEQ(20);

    testDiag("Done");
}
