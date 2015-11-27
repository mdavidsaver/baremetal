/* Test exception escalation in armv7-m
 */
#include "armv7m.h"

static
volatile int test;

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
    run_table.hard = hard;
    run_table.pendsv = pendsv;
    run_table.svc = svc;

    out32(SCB(0xd0c), 0x05fa0000 | (PRIGROUP<<8));

    test = 0;
    CPSIE(if);
    puts("1. trigger SVC\n");
    SVC(42);
    puts("3. Back in main\n");

    test = 1;
    puts("4. trigger PendSV\n");
    out32((void*)0xe000ed04, 1<<28);
    puts("6. Back in main\n");

    test = 2;
    puts("7. trigger HardFault\n");
    CPSID(i); /* mask prio lower than -1 [0,255] */
    SVC(42);
    puts("9. Back in main\n");

    test = 3;
    CPSIE(i);
    basepri(1);

    out32(SCB(0xd1c), PRIO(7,0)<<24); /* PRIO SVC */
    //out32(SCB(0xd1c), 1<<21); /* PRIO PendSV */

    puts("10. trigger HardFault\n");
    SVC(42);
    puts("12. Back in main\n");

    test = 4;
    out32(SCB(0xd1c), PRIO(0,0)<<24); /* PRIO SVC */
    puts("13. trigger SVC\n");
    SVC(42);
    puts("15. Back in main\n");

    test = 5;
    out32(SCB(0xd1c), PRIO(2,0)<<24); /* PRIO SVC */
    basepri(2);
    puts("16. trigger HardFault\n");
    SVC(42);
    puts("18. Back in main\n");

    puts("Done\n");
}
