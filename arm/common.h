/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define NELEM(X) (sizeof(X)/sizeof(X[0]))

/* mask with lower N bits set */
#define BMASK(N) ((1ull<(N))-1)
/* mask with bits m through N set (zero indexed)
 * BMASK(31,0)==0xffffffff
 * BMASK(23,16)==0x00ff0000
 */
#define BMASK2(M,N) (BMASK((M)+1)-BMASK(N))

typedef void(*isrfunc)(unsigned);

extern uint32_t RamSize;

/* Shutdown the system from init.S */
void halt(void);

/* from common.c */
void memcpy(void *dst, const void *src, size_t count);
void memset(void *dst, uint8_t val, size_t count);

int processATAG(uint32_t*);
uint32_t board_id;
extern const char *cmd_line;

/* from printk.c */
void puthex(uint32_t v);
void putdec(int v);
void putudec(unsigned v);
void putchar(char c);
void vprintk(unsigned i, const char *fmt, va_list args) __attribute__((format(printf,2,0)));
void printk(unsigned i, const char *fmt, ...) __attribute__((format(printf,2,3)));

/* from irq.c */
void irq_setup(void);
int isr_install(unsigned vect, isrfunc fn);
int isr_enable(unsigned vect);
int isr_disable(unsigned vect);

/* register bases */
#define A9_SYSCTRL_BASE ((volatile void*)0x10000000u)

#define A9_UART_BASE_1  ((volatile void*)0x10009000u)

/* interrupt controller */
#define A9_PIC_CPU_SELF ((volatile void*)0x1e000100u)
#define A9_PIC_CONF     ((volatile void*)0x1e001000u)

/* ARM sp804 */
#define A9_TIMER_BASE_1 ((volatile void*)0x10011000u)

/* byte order swap */

static inline __attribute__((always_inline,unused))
uint32_t swap32(uint32_t v)
{
    __asm__ ( "rev %1, %1" : "+r" (v) : : );
    return v;
}

static inline __attribute__((always_inline,unused))
uint16_t swap16(uint16_t v)
{
    __asm__ ( "rev16 %1, %1" : "+r" (v) : : );
    return v;
}

/* MMIO access */

static inline __attribute__((always_inline,unused))
void out32(volatile void *addr, uint32_t val)
{
    *(volatile uint32_t*)addr = val;
}
static inline __attribute__((always_inline,unused))
void out16(volatile void *addr, uint16_t val)
{
    *(volatile uint16_t*)addr = val;
}
static inline __attribute__((always_inline,unused))
void out8(volatile void *addr, uint8_t val)
{
    *(volatile uint8_t*)addr = val;
}

static inline __attribute__((always_inline,unused))
uint32_t in32(volatile void *addr)
{
    return *(volatile uint32_t*)addr;
}
static inline __attribute__((always_inline,unused))
uint32_t in16(volatile void *addr)
{
    return *(volatile uint16_t*)addr;
}
static inline __attribute__((always_inline,unused))
uint32_t in8(volatile void *addr)
{
    return *(volatile uint8_t*)addr;
}

unsigned irq_mask(void);
void irq_unmask(unsigned m);

#endif // COMMON_H
