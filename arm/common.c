/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM vexpress-a9 emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */

#include "common.h"

void memcpy(void *dst, const void *src, size_t count)
{
    char *cdst = dst;
    const char *csrc = src;
    while(count--)
        *cdst++ = *csrc++;
}

void memset(void *dst, uint8_t val, size_t count)
{
    char *cdst = dst;
    while(count--)
        *cdst++ = val;
}

void _assert_fail(const char *cond,
                  const char *file,
                  unsigned int line)
{
    printk(0, "%s:%u assertion fails: %s\n",
           file, line, cond);
    halt();
}
