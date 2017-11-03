
#include "common.h"

extern char __bss_start, __bss_end;
extern char __sbss_start, __sbss_end;
extern char __sbss2_start, __sbss2_end;

extern char __data_start, __data_end, __data_load;
extern char __sdata_start, __sdata_end, __sdata_load;
extern char __sdata2_start, __sdata2_end, __sdata2_load;

void prepare_data_segments(void)
{
    memset(&__bss_start, 0, &__bss_end-&__bss_start);
    memset(&__sbss_start, 0, &__bss_end-&__sbss_start);
    memset(&__sbss2_start, 0, &__bss_end-&__sbss2_start);

    memmove(&__data_start, &__data_load, &__data_end-&__data_start);
    memmove(&__sdata_start, &__sdata_load, &__sdata_end-&__sdata_start);
    memmove(&__sdata2_start, &__sdata2_load, &__sdata_end-&__sdata2_start);
}
