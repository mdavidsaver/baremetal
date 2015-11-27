/* Test IRQ masking and priorities in armv7-m
 */

// This test needs at least 1 bit sub-priority on TM4C
#define PRIGROUP 5

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

static int test;

static
void irq0(void)
{
    switch(test) {
    case 1:
        puts("4. in IRQ0\n");
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 0, in32(SCB(0x200)));
        test_equal("ACT ", 1, in32(SCB(0x300)));
        test_equal("ICSR", 0x00000810, in32(SCB(0xd04)));
        break;
    case 3:
        puts("8. in IRQ0\n");
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 0, in32(SCB(0x200)));
        test_equal("ACT ", 1, in32(SCB(0x300)));
        test_equal("ICSR", 0x00000810, in32(SCB(0xd04)));
        break;
    case 4:
        test = 5;
        puts("11. in IRQ0, pend IRQ1\n");
        __asm__ ("dsb" ::: "memory");
        out32(SCB(0x200), 2);
        puts("13. back in IRQ0\n");
        break;
    case 7:
        puts("18. in IRQ0\n");
        break;
    case 9:
        puts("22. in IRQ0\n");
        break;
    case 10:
        puts("25. in IRQ0\n");
        test = 11;
        __asm__ ("dsb" ::: "memory");
        break;
    default:
        puts("Fail IRQ0\n");
        abort();
    }
}

static
void irq1(void)
{
    switch(test) {
    case 2:
        puts("7. in IRQ1\n");
        test = 3;
        test_equal("ENA ", 3, in32(SCB(0x100)));
        test_equal("PEND", 1, in32(SCB(0x200)));
        test_equal("ACT ", 2, in32(SCB(0x300)));
        /* ISRPENDING, VECTPENDING==16, RETTOBASE, and VECACTIVE=17 */
        test_equal("ICSR", 0x00410811, in32(SCB(0xd04)));
        break;
    case 5:
        puts("12. in IRQ1\n");
        break;
    case 6:
        puts("16. in IRQ1, pend IRQ0\n");
        out32(SCB(0x200), 1);
        __asm__ ("dsb" ::: "memory");
        puts("17. still in IRQ1\n");
        test = 7;
        __asm__ ("dsb" ::: "memory");
        break;
    case 8:
        puts("21. in IRQ1\n");
        test = 9;
        __asm__ ("dsb" ::: "memory");
        break;
    case 11:
        puts("26. in IRQ1\n");
        break;
    default:
        puts("Fail IRQ1\n");
        abort();
    }
}

void main(void)
{
    run_table.irq[0] = irq0;
    run_table.irq[1] = irq1;

    out32(SCB(0xd0c), 0x05fa0000 | (PRIGROUP<<8));

    CPSID(i);

    test = 0;
    puts("1. Enable IRQ0/1\n");
    out32(SCB(0x100), 3);

    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 0, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));
    test_equal("ICSR", 0x00000000, in32(SCB(0xd04)));

    puts("2. Pend IRQ0 (shouldn't run)\n");
    out32(SCB(0x200), 1);
    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 1, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));
    /* ISRPENDING and VECTPENDING==16 */
    test_equal("ICSR", 0x00410000, in32(SCB(0xd04)));

    test = 1;
    puts("3. Unmask (should run now)\n");
    CPSIE(i);

    puts("5. Back in main\n");
    test_equal("ENA ", 3, in32(SCB(0x100)));
    test_equal("PEND", 0, in32(SCB(0x200)));
    test_equal("ACT ", 0, in32(SCB(0x300)));

    test = 2;
    out32(SCB(0x400), (PRIO(1,0)<<8)|PRIO(2,0)); /* Give IRQ1 priority over IRQ0 */

    puts("6. Pend IRQ0 and IRQ1 (should run now)\n");
    out32(SCB(0x200), 3);

    puts("9. Back in main\n");

    /* see that one handler can really pre-empt another */
    test = 4;
    puts("10. Pend IRQ0 (should run now)\n");
    out32(SCB(0x200), 1);

    puts("14. Back in main\n");

    /* see that one handler doesn't pre-empt another
     * when it has lower priority
     */
    test = 6;
    puts("15. Pend IRQ1 (should run now)\n");
    out32(SCB(0x200), 2);

    puts("19. Back in main\n");

    /* check sub-group ordering */
    test = 8;
    out32(SCB(0x400), (PRIO(0,0)<<8)|PRIO(0,0x3f)); /* equal prio, IRQ1 has lower sub-group */
    puts("20. Pend IRQ0 and IRQ1 (should run now)\n");
    out32(SCB(0x200), 3);

    puts("23. Back in main\n");

    /* check fallback it vector # */
    test = 10;
    out32(SCB(0x400), (PRIO(0,0)<<8)|PRIO(0,0)); /* equal prio, and sub-group, IRQ0 has lower vector */
    puts("24. Pend IRQ0 and IRQ1 (should run now)\n");
    out32(SCB(0x200), 3);

    puts("27. Back in main\n");

    puts("Done\n");
}
