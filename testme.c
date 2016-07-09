
#include "armv7m.h"
#include "testme.h"

static
volatile unsigned testcount;

#define CNT() __atomic_add_fetch(&testcount, 1, __ATOMIC_RELAXED)

void testInit(unsigned ntests)
{
    unsigned zero=0;
    __atomic_store(&testcount, &zero, __ATOMIC_RELAXED);
    if(ntests) {
        puts("1..");
        putdec(ntests);
    } else
        puts("# no test spec");
    putc('\n');
}

void testOk(int c, const char *msg)
{
    if(!c)
        puts("not ");
    puts("ok ");
    putdec(CNT());
    puts(" - ");
    puts(msg);
    putc('\n');
}

void testPass(const char *msg)
{
    testOk(1, msg);
}

void testFail(const char *msg)
{
    testOk(0, msg);
}

void testDiag(const char *msg)
{
    puts("# ");
    puts(msg);
    putc('\n');
}

void testEqI(uint32_t expect, uint32_t actual, const char *msg)
{
    if(expect!=actual) puts("not ");
    puts("ok ");
    putdec(CNT());
    puts(" - ");
    puthex(expect);
    puts(" == ");
    puthex(actual);
    putc(' ');
    puts(msg);
    putc('\n');
}
