/* just print some stuff to show that
 * things are initialized properly
 */
#include "armv7m.h"
#include "testme.h"

static uint32_t var1;
static uint32_t var2 = 0x87654321;

void main(void)
{
    testInit(2);
    testDiag("Starting");
    puts("# ");
	puthex(0x12345678);
    puts("\n# ");
    putdec(23456789);
    puts("\n# ");
    putdec(6789);
    puts("\n# ");
    putdec(600789);
    putc('\n');
    testEqI(0, var1, "var1");
    testEqI(0x87654321, var2, "var2");
}
