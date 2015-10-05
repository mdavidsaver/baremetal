/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"


/* some variables to see if we're loading up the data and bss sections correctly */
volatile uint32_t ioportbase;
uint32_t foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0x1badface;

static const uint32_t roval = 0x12345678;

static double val = 42.0;

void Init(void)
{
    printk(0, "hello world!\n");

    /* check that .bss and .data are setup correctly */
    printk(0, "ioportbase %x expect zero\n", (unsigned)ioportbase);
    printk(0, "foobar %x expect zero\n", (unsigned)foobar);
    printk(0, "ioportbase2 0xdeadbeef %x\n", (unsigned)ioportbase2);
    printk(0, "foobar2 0x1badface %x\n", (unsigned)foobar2);
    printk(0, "roval 0x12345678 %x\n", (unsigned)roval);

    {
        static int I[] = {0, 1, -1, 10, -10, 11, -11, 503, -203};
        unsigned i;

        printk(0, "Test printing signed decimal\n");
        for(i=0; i<NELEM(I); i++)
        {
            putdec(I[i]);
            putchar('\n');
        }
    }

    printk(0, "Unsigned %u\n", (unsigned)-1);

    printk(0, "Hello %s!\n", "world");

    {
        double a=val*2;
        printk(0, "Test printing double\n");
        printk(0, "double 0.0 %f\n", 0.0);
        printk(0, "double 1.0 %f\n", 1.0);
        printk(0, "double 101.0 %f\n", 101.0);
        printk(0, "double 84.0 %f\n", a);
    }

    printk(0, "Done\n");
}
