/* Capture initial register states
 */
#include "armv7m.h"

char _main_stack_top;

static
void test_equal(const char *msg, uint32_t lhs, uint32_t rhs)
{
    puts(lhs==rhs ? "ok - " : "fail - ");
    puthex(lhs);
    puts(" == ");
    puthex(rhs);
    puts(" # ");
    puts(msg);
    putc('\n');
}

struct early_state_t {
    uint32_t LR;
    uint32_t XPSR;
    uint32_t PRIMASK;
    uint32_t FAULTMASK;
    uint32_t BASEPRI;
    uint32_t CONTROL;
    uint32_t MSP;
    uint32_t PSP;
    uint32_t cpuid;
    uint32_t icsr;
    uint32_t vtor;
    uint32_t aircr;
    uint32_t scr;
    uint32_t ccr;
    uint32_t shpr[3];
    uint32_t shcsr;
    uint32_t syst_csr;
    uint32_t ictr;
    uint32_t mpu_type;
    uint32_t mpu_ctrl;
    uint32_t marker;
} early_state;

void main(void)
{
#define TEST(FLD, VAL) test_equal(#FLD, early_state.FLD, VAL)
    TEST(marker, 0xdeadbeaf); /* check consistency w/ init-m-test9.S */
    TEST(LR, 0xffffffff);
    TEST(XPSR, 0);
    TEST(PRIMASK, 0);
    TEST(FAULTMASK, 0);
    TEST(BASEPRI, 0);
    TEST(CONTROL, 0);
    TEST(MSP, (uint32_t)&_main_stack_top);
    TEST(PSP, 0);
    puts("# cpuid ");
    puthex(early_state.cpuid);
    putc('\n');
    TEST(icsr, 0);
    TEST(vtor, 0);
    TEST(aircr, 0xfa050000);
    TEST(scr, 0);
    TEST(ccr, 1<<9); /* STKALIGN */
    TEST(shpr[0], 0);
    TEST(shpr[1], 0);
    TEST(shpr[2], 0);
    TEST(shcsr, 0);
    TEST(syst_csr, 0);
    puts("# ictr ");
    puthex(early_state.ictr);
    putc('\n');
    puts("# mpu_type ");
    puthex(early_state.mpu_type);
    putc('\n');
    TEST(mpu_ctrl, 0);
    puts("Done\n");
}
