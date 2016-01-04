/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "kernel.h"
#include "user.h"
#include "bsp.h"

#ifdef DEF_RAM_SIZE
uint32_t RamSize = DEF_RAM_SIZE;
#else
uint32_t RamSize;
#endif

void memcpy(void *dst, const void *src, size_t count)
{
    char *cdst = dst;
    const char *csrc = src;
    while(count--)
        *cdst++ = *csrc++;
}

void memmove(void *dst, const void *src, size_t count)
{
    char *cdst = dst;
    const char *csrc = src;
    if(cdst==csrc) return;
    else if(cdst>csrc) {
        /* copy from back to front */
        cdst += count;
        csrc += count;
        while(count--)
            *--cdst = *--csrc;
    } else { /* cdst<csrc */
        /* copy from front to back */
        while(count--)
            *cdst++ = *csrc++;
    }
}

void memset(void *dst, uint8_t val, size_t count)
{
    char *cdst = dst;
    while(count--)
        *cdst++ = val;
}

int strcmp(const char *A, const char *B)
{
    int ret;
    char a, b;
    do {
        int lt, eq;
        a = *A++;
        b = *B++;
        lt = a < b;
        eq = a==b;
        ret = lt ? -1 : (eq ? 0 : 1);
    } while(a && b);
    return ret;
}

/* floor(log(v, 2))
 *  log2(31) -> 5
 *  log2(32) -> 6
 *  log2(33) -> 6
 */
unsigned log2_floor(uint32_t v)
{
    unsigned r=0;
    while(v) {
        v>>=1;
        r++;
    }
    return r;
}

/* ceil(log(v, 2))
 *  log2_ceil(31) -> 6
 *  log2_ceil(32) -> 6
 *  log2_ceil(33) -> 7
 */
unsigned log2_ceil(uint32_t v)
{
    unsigned r=0, c=0;
    while(v) {
        c += v&1;
        v >>= 1;
        r++;
    }
    if(c>1) r++;
    return r;
}

void _assert_fail(const char *cond,
                  const char *file,
                  unsigned int line)
{
    uint32_t control = 0;
    __asm__ ("mrs %0, CONTROL" : "=r"(control)::);
    if(control==3) { /* thread mode and unpriv */
        printf("%s:%u assertion fails: %s\n",
               file, line, cond);
    } else { /* handler or priv */
        printk("%s:%u assertion fails: %s\n",
               file, line, cond);
    }
    halt();
}
