
#include "armv7m.h"

const char hexchars[] = "0123456789abcdef";

void board_setup(void)
{
    if((in32(SCB(0xd00))&0xff0ffff0)==0x410fc240) {
        /* Assume EK-TM4C1294XL */
        /* From POR SysClk==PIOSC (16 Mhz) */

        //rmw(32, SYSCON(0x138), 0xf, 0); // ALTCLKCFG (0 is default)

        /* power up perphrials */
        rmw(32, SYSCON(0x618), 0x1, 0x1); // RCGCUART
        rmw(32, SYSCON(0x608), 0x1, 0x1); // RCGCGPIO

        loop_until(32,SYSCON(0xa18),1,==,1); // PRUART
        loop_until(32,SYSCON(0xa08),1,==,1); // PRGPIO

        /* setup gpio */
        rmw(32, GPIO(0,0x420), 3, 3); // AFSEL
        rmw(32, GPIO(0,0x51c), 3, 3); // DEN
        rmw(32, GPIO(0,0x52c), 0xff, 0x11); // PCTL
        rmw(32, GPIO(0,0x528), 3, 0); // AMSEL (0 is default)

        /* setup uart */
        rmw(32, UART(0x30), 1, 0); // UARTCTL

        out32(UART(0x24), 8); /* IBRD baud 115200 w/ 16 MHz ref. clock */
        out32(UART(0x28), 44); // FBRD

        out32(UART(0x2c), 0x60); // LCRH (order is import IBRD/FBRD then LCRH)

        rmw(32, UART(0xfc8), 0xf, 5); // UARTCC (use alt. clock)

        rmw(32, UART(0x30), 0x20, 0); // UARTCTL
        rmw(32, UART(0x30), 1, 1);

        /* setup system clock as MOSC+PLL */
        /* Power up MOSC for crystal at >= 10 MHz */
        rmw(32, SYSCON(0x7c), 0x1f, 0x10);

        loop_until(32,SYSCON(0x50), 1<<8,==, 1<<8); /* MOSCPUPIM ready */

        rmw(32, SYSCON(0x7c), 0x3, 0x3); /* enable MOSC validation, failure gives IRQ */

        if(in32(SYSCON(0x50))&0x08) {
            puts("MOSC startup fails\n");
        } else {
            uint32_t val;
            /* Switch clock and PLL sources to MOSC w/ no divider (/1).
             * SysClk now 25 Mhz.
             */
            out32(SYSCON(0xb0), (3<<24)|(3<<20));

            /* Max. sys clock frequency is 120 MHz.  We select 100 MHz.
             * f_xtal = 25 Mhz
             * PLL config is N=4, Q=0 (f_in=5 MHz)
             * MDIV = 50 (f_vco=100 MHz)
             * PSYSDIV=0 (SysClk=100 Mhz)
             */
            rmw(32, SYSCON(0x160), 0x008fffff, 50 | (1<<23)); /* MDIV | PLLPWR */
            rmw(32, SYSCON(0x164), 0x1f1f, 4); /* N | Q */
            rmw(32, SYSCON(0x0b0), 1<<30, 1<<30); /* NEWFREQ */

            loop_until(32, SYSCON(0x168), 1,==,1); /* wait for PLL lock */

            /* memory timing for 80<SysClk<=100 */
            val = 4 | (5<<6);
            val |= val<<16;
            rmw(32, SYSCON(0xc0), 0x03ef03ef, val);

            /* Switch clock source to PLL,.
             * Other settings as above
             */
            rmw(32, SYSCON(0x0b0), (1<<31)|(1<<28), (1<<31)|(1<<28)); /* MEMTIMU and USEPLL */
            /* SysClk now 100 MHz */
        }

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
