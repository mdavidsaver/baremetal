
#include "bsp.h"
#include "user.h"

void Init(void)
{
    int done = 10;
    printf("Init Starting\n");

    if(thread_start("Tick2", 0))
        printf("Failed to start Tick2\n");

    while(done--) {
        flush();
        msleep(1000);
        printf("Init %d\n", done);
    }
    printf("Init Finished\n");
    flush();
}

void Tick2(void)
{
    int done = 5;
    printf("Tick2 Starting\n");
    flush();

    while(done--) {
        msleep(2000);
        printf("Tick2 %d\n", done);
        flush();
    }
    printf("Tick2 Finished\n");
    flush();
}
