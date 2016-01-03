
#include "user.h"


/* some variables to see if we're loading up the data and bss sections correctly */
volatile uint32_t ioportbase;
uint32_t foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0x1badface;

static const uint32_t roval = 0x12345678;

static double val = 42.0;

void show_arm_id(void);

void Init(void)
{
    printf("hello world!\n");
    show_arm_id();

    /* check that .bss and .data are setup correctly */
    printf("ioportbase %x expect zero\n", (unsigned)ioportbase);
    printf("foobar %x expect zero\n", (unsigned)foobar);
    printf("ioportbase2 0xdeadbeef %x\n", (unsigned)ioportbase2);
    printf("foobar2 0x1badface %x\n", (unsigned)foobar2);
    printf("roval 0x12345678 %x\n", (unsigned)roval);

    {
        static int I[] = {0, 1, -1, 10, -10, 11, -11, 503, -203};
        unsigned i;

        printf("Test printing signed decimal\n");
        for(i=0; i<NELEM(I); i++)
        {
            printf("%d\n", I[i]);
        }
    }

    printf("Unsigned %u\n", (unsigned)-1);

    printf("Hello %s!\n", "world");

    {
        double a=val*2;
        printf("Test printing double\n");
        printf("double 0.0 %f\n", 0.0);
        printf("double 1.0 %f\n", 1.0);
        printf("double 101.0 %f\n", 101.0);
        printf("double 84.0 %f\n", a);
    }

    printf("Done\n");
    flush();
    halt();
}
