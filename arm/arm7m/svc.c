
#include "common.h"
#include "process.h"
#include "uart.h"

static inline __attribute__((always_inline))
uint32_t user_in32(void *addr)
{
    uint32_t ret;
    __asm__("ldrt %0, [%1]" : "=r"(ret) : "r"(addr));
    return ret;
}

static inline __attribute__((always_inline))
uint8_t user_in8(void *addr)
{
    uint8_t ret;
    __asm__("ldrbt %0, [%1]" : "=r"(ret) : "r"(addr));
    return ret;
}

typedef int (*svc_fn)(uint32_t *frame);

static
int svc_halt(uint32_t *frame)
{
    //TODO abort() process only
    halt();
    return -1; // never reached
}

static
int svc_yield(uint32_t *frame)
{
    thread_yield(thread_scheduler[0]);
    return 0;
}

static
int svc_uart(uint32_t *frame)
{
    int nput = 0;
    char c;
    char *buf = (char*)frame[FRAME_R0];
    uint32_t flags = frame[FRAME_R1];
    if(buf) {
        while((c=user_in8(buf++))!='\0') {
            uart_putc(c);
            nput++;
        }
    }
    if(flags&1) {
        uart_flush();
    }
    return nput;
}

static
svc_fn svc_calls[] = {
    &svc_halt,
    &svc_yield,
    &svc_uart,
};

void svc_handler_c(uint32_t *frame)
{
    unsigned ret = -127; // bad SVC
    uint16_t *pc, inst;
    uint8_t idx;

    pc = (void*)frame[FRAME_PC];
    inst = *(pc-1);

    assert((inst&0xff00)==0xdf00);

    idx = inst&0xf;
    if(idx<NELEM(svc_calls))
        ret = svc_calls[idx](frame);
    frame[FRAME_R0] = ret;
}
