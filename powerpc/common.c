
#include "common.h"

void memset(void *addr, int val, uint32_t cnt)
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
