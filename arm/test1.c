/* just print some stuff to show that
 * things are initialized properly
 */
#include "armv7m.h"

static uint32_t var1;
static uint32_t var2 = 0x87654321;

void main(void)
{
	puts("Starting: ");
	puthex(0x12345678);
	puts("\nvar1: ");
	puthex(var1);
	puts("\nvar2: ");
	puthex(var2);
	puts("\nDone\n");
}
