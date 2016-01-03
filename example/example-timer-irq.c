
#include "bsp.h"
#include "user.h"

void Init(void)
{
    int done = 10;
    printf("Starting\n");
    flush();

    while(done--) {
        msleep(1000);
        printf("Tick %d\n", done);
        flush();
    }
    printf("Finished\n");
    flush();
    halt();
}

