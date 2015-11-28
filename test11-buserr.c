/* See how various faults are reported and recovered
 */
#include "armv7m.h"

void inst_skip(uint32_t *sp);

static
volatile unsigned fault_type;

static
void check_fault(unsigned expect)
{
    unsigned actual = fault_type;
    puts("# Fault: ");
    switch(actual) {
    case 0: puts("None\n"); break;
    case 1: puts("Mem\n"); break;
    case 2: puts("Bus\n"); break;
    case 3: puts("Usage\n"); break;
    default: puthex(actual);
    }

    if(expect!=actual)
        puts("not ");
    puts("ok - ");
    puthex(actual);
    puts(" = ");
    puthex(expect);
    putc('\n');
    fault_type = 0;
}

static
void set_fault(unsigned actual)
{
    if(fault_type) {
        puts("Secondary fault ");
        puthex(actual);
        putc('\n');
    } else
        fault_type = actual;
}

static
void hard(void)
{
    puts("Unexpected HardFault\n");
    abort();
}

void bus(uint32_t *sp)
{
    uint32_t sts, addr;

    set_fault(2);

    sts = in32(SCB(0xd28));
    addr= in32(SCB(0xd38));

    out32(SCB(0xd28), 0xff00); /* W1C */
    out32(SCB(0xd38), 0);

    puts("BusFault: ");
    puthex(sts);
    putc(' ');
    if(sts&0x8000) {
        puthex(addr);
        putc(' ');
    }
    if(sts&0x1000)
        puts("STKERR ");
    if(sts&0x0800)
        puts("UNSTKERR ");
    if(sts&0x0400)
        puts("IMPRECISERR ");
    if(sts&0x0200)
        puts("PRECISERR ");
    if(sts&0x0100)
        puts("IBUSERR ");

    putc('\n');

    if(sts&0x0300) {
        /* precise faults would return to the faulting
         * instruction, which would then fault again
         * since we change nothing, so skip it.
         */
        puts("From: ");
        puthex(sp[6]);
        if(sp[6]<0xfffffff0)
            inst_skip(sp);
    } else {
        puts("From before: ");
        puthex(sp[6]);
    }
    putc('\n');

    if(sp[6]>=0xf0000000) {
        /* evil hack since we know this was a bx instruction */
        sp[6] = sp[5]&~1; /* jump to LR */
    }
}

static __attribute__((naked))
void bus_entry(void)
{
    asm("mov r0, sp");
    asm("b bus");
}

void mem(uint32_t *sp)
{
    uint32_t sts, addr;

    set_fault(1);

    sts = in32(SCB(0xd28));
    addr= in32(SCB(0xd34));

    out32(SCB(0xd28), 0xff); /* W1C */

    puts("MemFault: ");
    puthex(sts);
    putc(' ');
    if(sts&0x80) {
        puthex(addr);
        putc(' ');
    }
    if(sts&0x08)
        puts("MSTKERR ");
    if(sts&0x04)
        puts("MUNSTKERR ");
    if(sts&0x02)
        puts("DACCVIOL ");
    if(sts&0x01)
        puts("IACCVIOL ");

    putc('\n');

    puts("From: ");
    puthex(sp[6]);
    putc('\n');

    if(sp[6]>=0xf0000000) {
        /* evil hack since we know this was a bx instruction */
        sp[6] = sp[5]&~1; /* jump to LR */
    } else {
        inst_skip(sp);
    }
}

static __attribute__((naked))
void mem_entry(void)
{
    asm("mov r0, sp");
    asm("b mem");
}


void usage(uint32_t *sp)
{
    uint32_t sts;

    set_fault(1);

    sts = in32(SCB(0xd28));

    out32(SCB(0xd28), 0xffff0000); /* W1C */

    puts("UsageFault: ");
    if(sts&0x2000000)
        puts("DIVBYZERO ");
    if(sts&0x1000000)
        puts("UNALIGNED ");
    if(sts&0x80000)
        puts("NOCP ");
    if(sts&0x40000)
        puts("INVPC ");
    if(sts&0x20000)
        puts("INVSTATE ");
    if(sts&0x10000)
        puts("UNDEFINSTR ");

    putc('\n');

    puts("From: ");
    puthex(sp[6]);
    putc('\n');

    if(sts&0x20000) {
        abort(); /* attempt to execute ARM mode */
    }

    inst_skip(sp);
}

static __attribute__((naked))
void usage_entry(void)
{
    asm("mov r0, sp");
    asm("b usage");
}

static __attribute__((naked))
void jumpff(uint32_t x)
{
    (void)x;
    /* evilness
     * 'blx' would fault before updating 'lr' anyway, so use 'bx'
     * to be clear.
     * We will return using the link register from our caller.
     */
    asm("bx r0");
    asm("jumpstuck: b jumpstuck");
}

void main(void)
{
    run_table.bus = bus_entry;
    run_table.mem = mem_entry;
    run_table.usage = usage_entry;
    run_table.hard = hard;

    out32(SCB(0xd24), 0x70000); /* Enable Bus, Mem, and Usage Faults */

    puts("# w/o MPU, hits background mapping\n");

    puts("1. Cause BusFault 0xe1000ff0\n");
    out32((void*)0xe1000ff0, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("2. Another BusFault 0x10000000\n");
    out32((void*)0x10000000, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("3. Another BusFault 0x01000000\n");
    out32((void*)0x01000000, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("4. Another BusFault 0xfffffffe\n");
    out32((void*)0xfffffffe, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("5.1 MemFault (jump to 0xf1000001)\n");
    jumpff(0xf1000001);
    check_fault(1);
    puts("Back in Main\n");

    puts("5. MemFault (jump to 0xfffffff9)\n");
    jumpff(0xfffffff9);
    check_fault(1);
    puts("Back in Main\n");

    puts("# Enable MPU, but not for privlaged"
         " still hits background mapping\n");
    enable_mpu(1,0,0);

    puts("6. Cause BusFault 0xe1000ff0\n");
    out32((void*)0xe1000ff0, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("7. Another BusFault 0x10000000\n");
    out32((void*)0x10000000, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("8. Another BusFault 0x01000000\n");
    out32((void*)0x01000000, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("9. Another BusFault 0xfffffffe\n");
    out32((void*)0xfffffffe, 0);
    check_fault(2);
    puts("Back in Main\n");

// this jump works, and TI has hidden so data here
//    puts("5. MemFault (jump to 0x01000001)\n");
//    jumpff(0x01000001);
//    check_fault(1);
//    puts("Back in Main\n");

    puts("10. MemFault (jump to 0xfffffff9)\n");
    jumpff(0xfffffff9);
    check_fault(1);
    puts("Back in Main\n");

    // ROM region is made larger than actual rom to pass through 0x05000000
    set_mpu(0, 0x00000000, 0x08000000, MPU_NORMAL|MPU_RORO);
    set_mpu(1, 0x20000000, 0x00080000, MPU_NORMAL|MPU_RWRW|MPU_XN);
    set_mpu(2, 0x4000c000, 0x00001000, MPU_DEVICE|MPU_RWRW|MPU_XN);
    set_mpu(3, 0xe000e000, 0x00001000, MPU_DEVICE|MPU_RWRW|MPU_XN);
    // Allow through access to unconnected address space
    set_mpu(4, 0xe100e000, 0x00001000, MPU_DEVICE|MPU_RWRW|MPU_XN);

    puts("# Enable MPU, including privlaged\n");
    enable_mpu(1,1,0);

    // MPU allows through, but no one is home == BusFault
    puts("11. Cause BusFault 0xe1000ff0\n");
    out32((void*)0xe1000ff0, 0);
    check_fault(2);
    puts("Back in Main\n");

    puts("12. Another MemFault 0x10000000\n");
    out32((void*)0x10000000, 0);
    check_fault(1);
    puts("Back in Main\n");

    // MPU allows through, no one is home, but still MemFault...?
    puts("13. Another MemFault 0x05000000\n");
    out32((void*)0x05000000, 0);
    check_fault(1);
    puts("Back in Main\n");

    puts("14. Another MemFault 0xfffffffe\n");
    out32((void*)0xfffffffe, 0);
    check_fault(1);
    puts("Back in Main\n");

// this jump works, and TI has hidden so data here
//    puts("5. MemFault (jump to 0x01000001)\n");
//    jumpff(0x01000001);
//    check_fault(1);
//    puts("Back in Main\n");

    puts("15. MemFault (jump to 0xfffffff9)\n");
    jumpff(0xfffffff9);
    check_fault(1);
    puts("Back in Main\n");
}
