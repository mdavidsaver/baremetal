/* Test IRQ masking and priorities in armv7-m
 */

// This test needs at least 1 bit sub-priority on TM4C
#define PRIGROUP 5

#include "armv7m.h"

static int test;

static
void irq0(void)
{
    switch(test) {
    case 1:
        puts("4. in IRQ0\n");
        if(in32(SCB(0x100))!=3) {
            puts("Fail, not enabled\n");
            abort();
        }
        if(in32(SCB(0x200))!=0) {
            puts("Fail, pending\n");
            abort();
        }
        if(in32(SCB(0x300))!=1) {
            puts("Fail, active\n");
            abort();
        }
        break;
    case 3:
        puts("8. in IRQ0\n");
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

    rmw(32,SCB(0xd0c),0x700, PRIGROUP<<8);

    __asm__ ("cpsid i" :::);
    __asm__ ("cpsie f" :::);

    test = 0;
    puts("1. Enable IRQ0/1\n");
    if(in32(SCB(0x100))!=0) {
        puts("Fail, enabled\n");
        abort();
    }
    out32(SCB(0x100), 3);
    if(in32(SCB(0x100))!=3) {
        puts("Fail, not enabled\n");
        abort();
    }
    if(in32(SCB(0x200))!=0) {
        puts("Fail, pending\n");
        abort();
    }
    if(in32(SCB(0x300))!=0) {
        puts("Fail, active\n");
        abort();
    }

    puts("2. Pend IRQ0 (shouldn't run)\n");
    out32(SCB(0x200), 1);
    if(in32(SCB(0x100))!=3) {
        puts("Fail, not enabled\n");
        abort();
    }
    if(in32(SCB(0x200))!=1) {
        puts("Fail, not pending\n");
        abort();
    }
    if(in32(SCB(0x300))!=0) {
        puts("Fail, active\n");
        abort();
    }

    test = 1;
    puts("3. Unmask (should run now)\n");
    __asm__ ("cpsie i;dsb" :::);

    puts("5. Back in main\n");
    if(in32(SCB(0x100))!=3) {
        puts("Fail, not enabled\n");
        abort();
    }
    if(in32(SCB(0x200))!=0) {
        puts("Fail, pending\n");
        abort();
    }
    if(in32(SCB(0x300))!=0) {
        puts("Fail, active\n");
        abort();
    }

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
