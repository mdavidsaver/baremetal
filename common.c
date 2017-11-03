
#include "common.h"

void memcpy(void *rdest, const void *rsrc, size_t cnt)
{
    char *dest = rdest;
    const char *src = rsrc;

    if(dest==src) return;

    while(cnt--)
        *dest++ = *src++;
}

void memmove(void *rdest, const void *rsrc, size_t cnt)
{
    char *dest = rdest;
    const char *src = rsrc;

    if(dest < src) {
        /* |--- dest ---|
         *     |--- src ---|
         *
         * Start from src[0]
         */

        while(cnt--)
            *dest++ = *src++;
    } else if(dest > src) {
        /*     |--- dest ---|
         * |--- src ---|
         *
         * Start from src[cnt-1]
         */

        dest += cnt;
        src += cnt;

        while(cnt--)
            *--dest = *--src;
    }
}

void memset(void *addr, int val, size_t cnt)
{
    char *ptr = addr;
    while(cnt--)
        *ptr = val;
}

int strcmp(const char *lhs, const char *rhs)
{
    for(; *lhs==*rhs && '\0'!=*lhs; lhs++, rhs++) {}
    if('\0'==*lhs && '\0'==*rhs)
        return 0;
    else if('\0'==*lhs)
        return -1;
    else
        return 1;
}
