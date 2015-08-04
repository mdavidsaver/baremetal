/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"

#define ATAG_DEBUG

uint32_t RamSize;

const char *cmd_line;
static char *cmd_line_buf[64];

void memcpy(void *dst, const void *src, size_t count)
{
    char *cdst = dst;
    const char *csrc = src;
    while(count--)
        *cdst++ = *csrc++;
}

int processATAG(uint32_t* info)
{
    uint32_t alen, atag;

    if(!info) return 1; /* no bootloader info */

    printk(0, "Board ID %d\n", (unsigned)board_id);

    while(1) {
        uint32_t *data;
        alen = info[0];
        atag = info[1];
        if(atag==0)
            break;
        data = &info[2];
        info += alen;

#ifdef ATAG_DEBUG
        printk(0, "ATAG 0x%x length %x\n", (unsigned)atag, (unsigned)alen);
        {
            unsigned i;
            for(i=0; i<alen-2; i++)
                printk(0, " %x", (unsigned)data[i]);
        }
        printk(0, "\n");
#endif

        switch(atag) {
        case 0x54410002: /* ATAG_MEM */
            RamSize = *data;
            break;
        case 0x54410009: /* ATAG_CMDLINE */
        {
            uint32_t cmdlen = 4*(alen-2);
            if(cmdlen>=sizeof(cmd_line_buf))
                cmdlen = sizeof(cmd_line_buf)-1;
            memcpy(cmd_line_buf, data, cmdlen);
            cmd_line = (char*)cmd_line_buf;
        }
            break;
        case 0x54410001: /* ATAG_CORE */
        case 0x54420005: /* ATAG_INITRD2 */
        case 0x414f4d50: /* ATAG_BOARD */
        default:
            break; /* ignore */
        }
    }
    return 0;
}
