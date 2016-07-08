/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
#include <common.h>

extern char __bss_start, __bss_end;
extern char __data_start, __data_end, __data_load;

/* some variables to see if we're loading up the data and bss sections correctly */
volatile uint32_t ioportbase;
uint32_t foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0x1badface;

static const uint32_t roval = 0x12345678;


void Init(uint32_t id, uint32_t *info)
{
    memset(&__bss_start, 0, &__bss_end-&__bss_start);
    memcpy(&__data_start, &__data_load, &__data_end-&__data_start);

    testInit(6);

    testeq32(foobar, 0);
    testeq32(foobar2, 0x1badface);
    testeq32(ioportbase, 0);
    testeq32(ioportbase2, 0xdeadbeef);
    testeq32(roval, 0x12345678);

    puts("# ID ");
    putval(id);
    puts("\r\n# PTR ");
    putval((uint32_t)info);
    puts("\r\n");

    if((id==0x8e0) ^ (info!=0))
        puts("not ");

    if(info && (info[1]&0x54400000)==0x54400000) {
        uint32_t alen, atag;
        puts("ok ");
        putval(++testcnt);
        puts(" - Have ATAG board info\r\n");

        while(1) {
            uint32_t *data;
            alen = info[0];
            atag = info[1];
            if(atag==0)
                break;
            data = &info[2];
            info += alen;

            puts("# ATAG 0x");
            putval(atag);
            puts(" length 0x");
            putval(alen);
            puts("\r\n");

            switch(atag) {
            case 0x54410002: /* ATAG_MEM */
                puts("# Ram size 0x");
                putval(*data);
                puts(" MB\r\n");
                break;
            case 0x54410009: /* ATAG_CMDLINE */
                puts("# Cmdline ");
                nputs((char*)data, 4*(alen-2));
                puts("\r\n");
                break;
            case 0x54410001: /* ATAG_CORE */
            case 0x54420005: /* ATAG_INITRD2 */
            case 0x414f4d50: /* ATAG_BOARD */
            default:
                break; /* ignore */
            }
        }


    } else {
        puts("ok ");
        putval(++testcnt);
        puts(" - No ATAG board info\r\n");
    }

}
