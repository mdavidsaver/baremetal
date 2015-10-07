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

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE (0x4000)
#define PAGE_MASK (PAGE_SIZE-1)
/* round address down to start of page */
#define _PAGE_START(A) (void*)( (size_t)(A)&~PAGE_MASK)
/* round address up to start of next page (if not already page aligned) */
#define _PAGE_UP(A) (void*)( 1+(((size_t)(A)-1)|PAGE_MASK))

#define NELEM(X) (sizeof(X)/sizeof(X[0]))
static inline
int _elem_in(void* base, void* elem, unsigned S, unsigned count)
{
    char *cbase=(char*)base, *celem=(char*)elem;
    unsigned idx;
    if(celem<cbase) return -1;
    idx = (celem-cbase)/S;
    if(idx>=count) return -1;
    else return idx;
}
#define INDEXOF(BASE, ELEM) _elem_in(BASE, ELEM, sizeof(BASE[0]), NELEM(BASE))

/* mask with lower N bits set */
#define BMASK(N) ((1ull<<(N))-1)
/* mask with bits m through N set (zero indexed)
 * BMASK2(31,0)==0xffffffff
 * BMASK2(23,16)==0x00ff0000
 */
#define BMASK2(M,N) (BMASK((M)+1)-BMASK(N))

#define EXTRACT(V, S, E) (((V)>>(S))&BMASK((E)-(S)+1))

/* Shutdown the system from init.S */
void halt(void);

/* from common.c */
void memcpy(void *dst, const void *src, size_t count);
void memmove(void *dst, const void *src, size_t count);
void memset(void *dst, uint8_t val, size_t count);
unsigned log2(uint32_t v);
unsigned log2_ceil(uint32_t v);

/* from atag.c */
int processATAG(uint32_t*);
extern uint32_t board_id;
extern uint32_t RamSize;
extern const char *cmd_line;

/* from page-alloc.c */
void page_alloc_setup(void);

/* from printk.c */
void puthex(uint32_t v);
void putdec(int v);
void putudec(unsigned v);
void putchar(char c);
void puts(const char *str);
void vprintk(unsigned i, const char *fmt, va_list args) __attribute__((format(printf,2,0)));
void printk(unsigned i, const char *fmt, ...) __attribute__((format(printf,2,3)));

/* from irq.c */

typedef void(*isrfunc)(unsigned);

void irq_setup(void);
void irq_show(void);
int isr_install(unsigned vect, isrfunc fn);
int isr_enable(unsigned vect);
int isr_disable(unsigned vect);
int isr_active(void);

unsigned irq_mask(void);
void irq_unmask(unsigned m);

/* from page-alloc.c */
void* page_alloc(void);
void page_free(void* addr);

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

#define assert(COND) do{if(COND) {} else {_assert_fail(#COND, __FILE__, __LINE__);}}while(0)
void _assert_fail(const char *cond,
                  const char *file,
                  unsigned int line);

#ifdef __cplusplus
}
#endif

#endif // COMMON_H
