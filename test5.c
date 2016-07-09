/* Test Mode/Stack changes
 */
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

extern char _main_stack_top, _proc_stack_top;
extern char _main_stack_bot, _proc_stack_bot;

static
void show_control(unsigned ectrl, unsigned evect)
{
    uint32_t actrl, avect;
    unsigned instack;
    char *sp;
    __asm__ ("mov %0,sp" : "=r"(sp) ::);
    __asm__ ("mrs %0,IPSR" : "=r"(avect) ::);
    __asm__ ("mrs %0,CONTROL" : "=r"(actrl) ::);
    testDiag("SP %p", sp);

    test_equal("CONTROL", ectrl, actrl);
    test_equal("IPSR", evect, avect);

    if(sp>&_main_stack_bot && sp<=&_main_stack_top)
        instack = 0;
    else if(sp>&_proc_stack_bot && sp<=&_proc_stack_top)
        instack = 2;
    else {
        testFail("fail - Corrupt SP %p", sp);
        return;
    }
    testOk((avect>0 && instack==0) || (avect==0 && (actrl&2)==instack),
           "stack%c, avect=%x instack=%x actrl=%x", instack?'2':'0',
           (unsigned)avect, (unsigned)instack, (unsigned)actrl);
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
    unsigned test = SEQ();
    printk("# in SVC SEQ %u\n", test);
    switch(test) {
    case 1:
        show_control(0, 11);
        break;
    case 3:
        show_control(0, 11);
        break;
    case 5:
        show_control(1, 11);
        break;
    case 7:
        show_masks(0);
        __asm__ volatile ("cpsid i" :::);
        show_masks(1);
        break;
    default:
        testFail("Fail SVC\n");
        abort();
    }
}

static
void hard(void)
{
    unsigned test = SEQ();
    testDiag("HARDFAULT %u", test);
    if(test==9) {
        testDiag("Set CONTROL=0");
        uint32_t val = 0;
        __asm__ volatile ("msr CONTROL, %0" :: "r"(val):);
    } else {
        testFail("Unexpected HardFault SEQ %u", test);
        abort();
    }
}

void main(void)
{
    run_table.svc = svc;
    run_table.hard = hard;

    testInit(45);
    
    {
        uint32_t temp = (uint32_t)&_proc_stack_top;
        __asm__ volatile ("msr PSP,%0" :: "r"(temp) :);
    }

    testDiag("MSP=%p PSP=%p", &_main_stack_top, &_proc_stack_top);

    show_control(0, 0);

    testDiag("Start, trigger SVC");
    CPSIE(if);
    SVC(42);

    testDiag("Back in main");
    CHECK_SEQ(2);
    show_control(0, 0);

    testDiag("Priv w/ proc stack");
    {
        uint32_t val = 2;
        __asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
    }
    show_control(2, 0);

    testDiag("trigger SVC");
    SVC(42);

    testDiag("Back in main");
    CHECK_SEQ(4);
    show_control(2, 0);

    testDiag("Drop privlage");
    {
        uint32_t val = 3;
        __asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
    }
    show_control(3, 0);

    testDiag("trigger SVC");
    SVC(42);

    testDiag("Back in main");
    CHECK_SEQ(6);
    show_control(3, 0);

    testDiag("Try to restore privlage and switch stack (should be noop)");
    {
        uint32_t val = 0;
        __asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
    }
    show_control(3, 0);

    testDiag("Try to set masks");
    CPSID(if);
    show_masks(0); /* doesn't work */

    testDiag("trigger SVC");
    SVC(42);

    testDiag("Back in main");
    CHECK_SEQ(8);
    show_masks(0); /* unprivlaged doesn't see mask */

    testDiag("trigger HardFault (restores priv)");
    SVC(42);
    testDiag("Back in main");
    CHECK_SEQ(10);
    show_control(2, 0);
    testDiag("restore MSP");
    {
        uint32_t val = 0;
        __asm__ volatile ("msr CONTROL,%0" :: "r"(val):);
    }
    show_control(0, 0);

    testDiag("Done.");
}
