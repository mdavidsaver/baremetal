
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

uint32_t ireg[32] __attribute__((section(".data.early")));

struct bd_info_t {
    uint32_t memstart, memsize;
    uint32_t flashstart, flashsize, flashoffset;
    uint32_t sramstart, sramsize;
    uint32_t mbarbase;
}bd_info;

/* MPC5200 reset value */
uint32_t MBAR = 0x80000000u;
#define PSC1 (MBAR+0x2000u)

uint16_t ioread16(uint32_t addr)
{
    volatile uint16_t *P = (void*)addr;
    asm volatile ("eieio" ::: "memory");
    return *P;
}

void iowrite8(uint8_t val, uint32_t addr)
{
    volatile uint8_t *P = (void*)addr;
    *P = val;
    asm volatile ("eieio" ::: "memory");
}

void uart_tx(char c)
{
    while(!(ioread16(PSC1+0x4)&(1<<11))) {}
    iowrite8(c, PSC1+0xc);
}

void putc(char c)
{
    if(c=='\n') uart_tx('\r');
    uart_tx(c);
}

void puts(const char *s)
{
    char c;
    while((c=*s++)!='\0')
        putc(c);
}

const char hexchars[] = "0123456789ABCDEF";

void puthex(uint32_t val)
{
    unsigned i;
    for(i=0; i<8; i++, val<<=4)
        uart_tx(hexchars[val>>28]);
}

void printk(const char *fmt, ...) __attribute__((format(__printf__,1,2)));
void vprintk(const char *fmt, va_list) __attribute__((format(__printf__,1,0)));

void vprintk(const char *fmt, va_list args)
{
    char c;
    while((c=*fmt++)!='\0') {
        switch(c) {
        case '\n':
            uart_tx('\r');
        default:
            uart_tx(c);
            break;
        case '%':
            c=*fmt++;
            switch(c) {
            case 's': {
                const char * v = va_arg(args, const char*);
                puts(v);
                break;
            }
            case 'x': {
                unsigned v = va_arg(args, unsigned);
                puthex(v);
                break;
            }
            case '\0':
                uart_tx('!');
                return;
            default:
                uart_tx('!');
            }
        }
    }
}

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}

void Init(void)
{
    unsigned i;
    {
        uint32_t *pdst = (uint32_t*)&bd_info,
                 *psrc = (uint32_t*)ireg[3];
        for(i=0; i<8; i++) {
            pdst[i] = psrc[i];
        }
    }
    /* use MBAR provided by u-boot */
    if(bd_info.mbarbase)
        MBAR = bd_info.mbarbase;
    puts("Hello world\nInitial register values:\n");
    for(i=0; i<32; i++) {
        puthex(ireg[i]);
        if(i%4==3)
            putc('\n');
        else
            putc(' ');
    }
    printk("memstart = %x\nmemsize = %x\n",
           (unsigned)bd_info.memstart,
           (unsigned)bd_info.memsize);
    printk("flashstart = %x\nflashsize = %x\nflashoffset = %x\n",
           (unsigned)bd_info.flashstart,
           (unsigned)bd_info.flashsize,
           (unsigned)bd_info.flashoffset);
    printk("MBAR = %x\n", (unsigned)bd_info.mbarbase);
    while(1) {}
}
