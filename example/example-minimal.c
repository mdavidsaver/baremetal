
#include "kernel.h"

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
    printk("hello world!\n");
    show_arm_id();

    /* check that .bss and .data are setup correctly */
    printk("ioportbase %x expect zero\n", (unsigned)ioportbase);
    printk("foobar %x expect zero\n", (unsigned)foobar);
    printk("ioportbase2 0xdeadbeef %x\n", (unsigned)ioportbase2);
    printk("foobar2 0x1badface %x\n", (unsigned)foobar2);
    printk("roval 0x12345678 %x\n", (unsigned)roval);

    {
        static int I[] = {0, 1, -1, 10, -10, 11, -11, 503, -203};
        unsigned i;

        printk("Test printing signed decimal\n");
        for(i=0; i<NELEM(I); i++)
        {
            printk("%d\n", I[i]);
        }
    }

    printk("Unsigned %u\n", (unsigned)-1);

    printk("Hello %s!\n", "world");

    {
        double a=val*2;
        printk("Test printing double\n");
        printk("double 0.0 %f\n", 0.0);
        printk("double 1.0 %f\n", 1.0);
        printk("double 101.0 %f\n", 101.0);
        printk("double 84.0 %f\n", a);
    }

    printk("Done\n");
}
