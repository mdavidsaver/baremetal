/* Minimal bare metal powerpc32 program for use without a bootloader.
 * Parts are specific to PPC PreP emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include <common.h>

extern char __bss_start, __bss_end;
extern char __sbss_start, __sbss_end;
extern char __sbss2_start, __sbss2_end;
extern char __data_start, __data_end, __data_load;
extern char __sdata_start, __sdata_end, __sdata_load;
extern char __sdata2_start, __sdata2_end, __sdata2_load;

/* some variables to see if we're loading up the data and bss sections correctly */
volatile char *ioportbase;
int foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0xbad1face;

static inline
void outb(uint16_t port, uint8_t val)
{
    /* I/O ports are memory mapped on PPC.  For PREP they begin at 0x80000000 */
    volatile uint8_t* addr = port+(uint8_t*)0x80000000;
    asm volatile ("eieio" ::: "memory");
    *addr = val;
}

void putchar(char c)
{
    outb(0x3f8, c);
}

uint8_t inb(uint16_t port)
{
    volatile uint8_t* addr = port+(uint8_t*)0x80000000;
    asm volatile ("eieio" ::: "memory");
    return *addr;
}

static __attribute__((unused))
uint8_t nvram_read(uint16_t addr)
{
    outb(0x74, addr);
    outb(0x75, addr>>8);
    return inb(0x77);
}

static __attribute__((unused))
void nvram_get_string(uint16_t addr, char *buf, size_t len)
{
    len--;
    while(len--) {
        char c = nvram_read(addr++);
        if(c=='\0')
            break;
        *buf++ = c;
    }
    *buf = '\0';
}

void Init(void)
{
    /* zero out BSS sections */
    memset(&__bss_start, 0, &__bss_end-&__bss_start);
    memset(&__sbss_start, 0, &__sbss_end-&__sbss_start);
    memset(&__sbss2_start, 0, &__sbss2_end-&__sbss2_start);
    /* copy from ROM to RAM */
    memcpy(&__data_start, &__data_load, &__data_end-&__data_start);
    memcpy(&__sdata_start, &__sdata_load, &__sdata_end-&__sdata_start);
    memcpy(&__sdata2_start, &__sdata2_load, &__sdata2_end-&__sdata2_start);
    /* setup the 16550 UART */
    outb(0x3fb, 3); /* 8N1, no break, normal access 0b00000011 */

    testInit(5);

    testeq32(foobar, 0);
    testeq32(foobar2, 0xbad1face);
    testeq32(ioportbase, 0);
    testeq32(ioportbase2, 0xdeadbeef);

    {
        /* QEMU communicates with us via an M48T59 NVRAM device.
         * Look for PPC_NVRAM_set_params() in QEMU source for full listing
         */
        char buf[32];
        nvram_get_string(0x00, buf, sizeof(buf));
        puts("# BIOS id string\r\n");
        if(strcmp(buf, "QEMU_BIOS")!=0) puts("not ");
        puts("ok ");
        putval(testcnt++);
        puts(" - '");
        puts(buf);
        puts("' == 'QEMU_BIOS'\r\n");
    }

    outb(0x64, 0xfe); /* ask the KBC to reset us */
    while(1) {}
}
