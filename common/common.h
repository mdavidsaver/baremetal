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

#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32

#define assert(COND) do{if(COND) {} else {_assert_fail(#COND, __FILE__, __LINE__);}}while(0)
void _assert_fail(const char *cond,
                  const char *file,
                  unsigned int line);

/* from common.c */
#ifndef __linux__
void memcpy(void *dst, const void *src, size_t count);
void memmove(void *dst, const void *src, size_t count);
void memset(void *dst, uint8_t val, size_t count);
#endif
unsigned log2_floor(uint32_t v) __attribute__((pure));
unsigned log2_ceil(uint32_t v) __attribute__((pure));

extern uint32_t RamSize;


#ifdef __cplusplus
}
#endif

#endif // COMMON_H
