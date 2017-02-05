
#include "common.h"

void memset(void *addr, int val, uint32_t cnt)
{
    char *ptr = addr;
    while(cnt--)
        *ptr = val;
}
