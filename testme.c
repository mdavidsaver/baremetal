
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

void testOk(int c, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    testVOk(c, msg, args);
    va_end(args);
}

void testVOk(int c, const char *msg, va_list args)
{
    if(!c)
        puts("not ");
    puts("ok ");
    putdec(CNT());
    puts(" - ");
    vprintk(msg, args);
    putc('\n');
}

void testPass(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    testVOk(1, msg, args);
    va_end(args);
}

void testFail(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    testVOk(0, msg, args);
    va_end(args);
}

void testDiag(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    puts("# ");
    vprintk(msg, args);
    putc('\n');
    va_end(args);
}

void testEqI(uint32_t expect, uint32_t actual, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    if(expect!=actual) puts("not ");
    puts("ok ");
    putdec(CNT());
    puts(" - ");
    puthex(expect);
    puts(" == ");
    puthex(actual);
    putc(' ');
    vprintk(msg, args);
    putc('\n');
    va_end(args);
}
