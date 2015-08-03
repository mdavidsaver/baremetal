/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __SIZE_TYPE__ size_t;

/* some variables to see if we're loading up the data and bss sections correctly */
volatile uint32_t ioportbase;
uint32_t foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0x1badface;

static const uint32_t roval = 0x12345678;

void xputs(const char*);
void xnputs(const char*, uint32_t);
void xputchar(char);
void halt(void);

static __attribute__((unused))
void putval(uint32_t v)
{
    static char hex[] = "0123456789ABCDEF";
    uint8_t n = sizeof(v)*2;

    while(n--) {
        xputchar(hex[v>>28]);
        v<<=4;
    }
}

void Init(uint32_t id, uint32_t *info)
{
    putval(id);
    xputs("  ");
    putval((uint32_t)info);
    xputs("\r\n");

    /* check that .bss and .data are setup correctly */
    putval(ioportbase); /* expect zero */
    xputs("\r\n");
    putval(foobar); /* expect zero */
    xputs("\r\n");
    putval(ioportbase2);
    xputs("\r\n");
    putval(foobar2);
    xputs("\r\n");
    putval(roval);
    xputs("\r\n");

    if(info && (info[1]&0x54400000)==0x54400000) {
        uint32_t alen, atag;
        xputs("Have ATAG board info\r\n");

        while(1) {
            uint32_t *data;
            alen = info[0];
            atag = info[1];
            if(atag==0)
                break;
            data = &info[2];
            info += alen;

            xputs("ATAG 0x");
            putval(atag);
            xputs(" length 0x");
            putval(alen);
            xputs("\r\n");

            switch(atag) {
            case 0x54410002: /* ATAG_MEM */
                xputs(" Ram size 0x");
                putval(*data);
                xputs(" MB\r\n");
                break;
            case 0x54410009: /* ATAG_CMDLINE */
                xputs(" Cmdline ");
                xnputs((char*)data, 4*(alen-2));
                xputs("\r\n");
                break;
            case 0x54410001: /* ATAG_CORE */
            case 0x54420005: /* ATAG_INITRD2 */
            case 0x414f4d50: /* ATAG_BOARD */
            default:
                break; /* ignore */
            }
        }


    } else {
        xputs("No ATAG board info\r\n");
    }

    /* defaults for PL011 UART work for QEMU */

    xputs("hello world!\r\n");
}
