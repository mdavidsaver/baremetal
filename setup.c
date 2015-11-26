
#include "armv7m.h"

void board_setup(void)
{
    if((in32(SCB(0xd00))&0xff0ffff0)==0x410fc240) {
        /* Assume EK-TM4C1294XL */

        rmw(32, SYSCON(0x138), 0xf, 0); // ALTCLKCFG (0 is default)

        rmw(32, SYSCON(0x618), 0x1, 0x1); // RCGCUART
        rmw(32, SYSCON(0x608), 0x1, 0x1); // RCGCGPIO

        loop_until(32,SYSCON(0xa18),1,==,1); // PRUART
        loop_until(32,SYSCON(0xa08),1,==,1); // PRGPIO

        rmw(32, GPIO(0,0x420), 3, 3); // AFSEL
        rmw(32, GPIO(0,0x51c), 3, 3); // DEN
        rmw(32, GPIO(0,0x52c), 0xff, 0x11); // PCTL
        rmw(32, GPIO(0,0x528), 3, 0); // AMSEL (0 is default)

        rmw(32, UART(0x30), 1, 0); // UARTCTL

        out32(UART(0x24), 8); /* IBRD baud 115200 */
        out32(UART(0x28), 44); // FBRD

        out32(UART(0x2c), 0x60); // LCRH

        rmw(32, UART(0xfc8), 0xf, 5); // UARTCC

        rmw(32, UART(0x30), 0x20, 0); // UARTCTL
        rmw(32, UART(0x30), 1, 1);

    } else {
        /* Assume lm3s6965evb */

        rmw(32, UART(0x30), 1, 0);
        out32(UART(0x24), 8); /* baud 115200 */
        out32(UART(0x28), 44);

        out32(UART(0x2c), 0x70);

        rmw(32, UART(0x30), 0x20, 0);
        rmw(32, UART(0x30), 1, 1);
    }
}
