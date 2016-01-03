
#include "arm7m.h"
#include "kernel.h"
#include "uart.h"
#include "systick.h"
#include "process.h"

void prepare_processes(void);

void bsp_setup_early(void) __attribute__((weak));
void uart_setup(void) __attribute((weak));
void bsp_setup(void) __attribute((weak));

void bsp_setup_early(void) {}
void uart_setup(void) {}
void bsp_setup(void) {}

static void nvic_setup(void);
static void mpu_setup(void);

/* multiplier for exception priorities */
uint8_t prio_mult;
uint8_t prio_min; /* lowest priority, unscaled (numerically highest) */

void setup_c(void)
{
    scs_out32(0xd14, (1<<9)|(1<<3)); /* CCR = STKALIGN | UNALIGN_TRP */

    bsp_setup_early();
    uart_setup();
    asm("cpsie f"); /* now the hardfault handler can tell us what goes wrong */

    scs_out32(0xd0c, 0x05fa0000); /* set PRIGROUP==0, one sub-group bit */

    {
        /* introspect the number of bits in the priority fields.
         * Many v7-M targets provide only the N MSBs.
         * the lower bits are ignored (read zero)
         */
        uint8_t val;
        scs_out32(0xd1c, 0xff000000);
        val = scs_in32(0xd1c)>>24;
        if(val&1) {
            /* wow, all bits available */
            prio_mult = 2; /* one sub-group bit */
            prio_min = 127;
        } else {
            unsigned n, v;
            for(n=0, v=val; val; n++, val<<=1) {}
            (void)v;
            prio_mult = (~val)+1;
            prio_min = (1<<n)-1;
        }
    }

    mpu_setup();
    nvic_setup();
    systick_setup();

    bsp_setup();
    __asm__ volatile ("dsb\n" "cpsie if" :::"memory");

    prepare_processes(); // never returns
}

unsigned irq_mask(void)
{
    unsigned m;
    __asm__("mrs %0, PRIMASK\ncpsid i\ndsb" : "=r"(m) :: "memory");
    return ~m; /* 1 if we set primask */
}

void irq_restore(unsigned m)
{
    if(m)
        __asm__("dsb\ncpsie i" ::: "memory");
}

static
isrfunc *exception_handlers;
static
unsigned num_exceptions;

extern isrfunc _boot_reset[16];

static
void nvic_enable(uint16_t offset, unsigned vect)
{
    uint32_t bit;

    offset = (vect&(~0x1f))>>3; /* /32*4 */
    bit = vect&0x1f;
    scs_out32(0x1fc+offset, 1<<bit);
}

static
void unhandled_interrupt(void)
{
    uint32_t vect;
    __asm__ ("mrs %0, IPSR" : "=r"(vect) ::);

    uart_puts("unhandled exception\n");
    nvic_enable(0x180, vect-16);
}

int isr_install(unsigned vect, isrfunc fn)
{
    unsigned mask = irq_mask();
    if(fn==NULL)
        fn = &unhandled_interrupt;
    exception_handlers[vect+16] = fn;
    irq_restore(mask);
    return 0;
}

int isr_enable(unsigned vect)
{
    nvic_enable(0x100, vect);
    return 0;
}

int isr_disable(unsigned vect)
{
    nvic_enable(0x180, vect);
    return 0;
}

static void nvic_setup(void)
{
    size_t i;
    uint32_t ictr = scs_in32(0x004);
    uint32_t nexcept = 16 + 32*(1+(ictr&0xf));

    num_exceptions = nexcept;

    /* allocate space for the runtime vector table based on the
     * advertised number of interrupts
     */
    exception_handlers = early_alloc(4*nexcept, 1<<7);

    memcpy(exception_handlers, _boot_reset, sizeof(_boot_reset));
    for(i=16; i<nexcept; i++)
        exception_handlers[i] = &unhandled_interrupt;

    asm("cpsid if":::"memory");
    scs_out32(0xd08, (uint32_t)exception_handlers);
    asm("dsb\ncpsid if":::"memory");

    /* No nested interrupts, leave all handlers run at priority 0 */

    /* Enable Bus, Mem, and Usage faults */
    scs_out32(0xd24, (1<<18)|(1<<17)|(1<<16));
}

static
void mpu_region(unsigned n, uint32_t base, uint32_t size, uint32_t attrs)
{
    unsigned lsize = log2_ceil(size)-2; // allow the background regions to be longer than necessary
    unsigned asize = 1<<(lsize+1);
    uint32_t bar = base | 0x10 | n,
             att = attrs | (lsize<<1) | 1;
    assert((base&0x1f)==0);
    assert((base&(asize-1))==0);

    printk("MPU%u BASE=%08x SIZE=%08x ATTRS=%08x\n", n, (unsigned)base, asize, (unsigned)att);
    scs_out32(0xd9c, bar);
    scs_out32(0xda0, att);
}

extern char __rom_start, __rom_end;
extern char __ram_start;

static void mpu_setup(void)
{
    uint32_t mputype = scs_in32(0xd90),
             nregions= (mputype>>8)&0xff,
             ramsize = RamSize;

    if(nregions==0) {
        printk("MPU not available\n");
        return;
    } else if(nregions<8) {
        printk("MPU insufficent regions %u\n", (unsigned)nregions);
        return;
    }
    printk("MPU supported\n");

    if(ramsize==0)
        ramsize = 0x10000000; // default to max. possible size

    /* setup background regions */
    mpu_region(0, (uint32_t)&__rom_start, &__rom_end-&__rom_start, MPU_AP_RORO|MPU_ROM);
    mpu_region(1, (uint32_t)&__ram_start, ramsize, MPU_XN|MPU_AP_RWRO|MPU_RAM);
    /* processes set regions 2-5 */
    /* region 6 is reserved */

    /* catch de-ref. NULL.  covers the initial vector table (moved to ram by this point) */
    mpu_region(7, 0, 64, MPU_XN|MPU_AP_NONO|MPU_ROM);

    scs_out32(0xd94, 5); /* MPU_CTRL = ENABLE | PRIVDEFENA */

}
