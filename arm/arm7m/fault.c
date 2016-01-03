
#include "kernel.h"
#include "cpu.h"

void hardfault_handler(void) {
    uint32_t hfsr = scs_in32(0xd2c);
    uint32_t cfsr = scs_in32(0xd28);
    printk("HardFault cfsr=%08x hfsr=%08x\n", (unsigned)cfsr, (unsigned)hfsr);
    halt();
}

void memfault_handler(void) {
    uint32_t mmfar = scs_in32(0xd34);
    uint32_t cfsr = scs_in32(0xd28);
    printk("MemFault cfsr=%08x mmfar=%08x\n", (unsigned)cfsr, (unsigned)mmfar);
    halt();
}

void busfault_handler(void) {
    uint32_t bfar = scs_in32(0xd38);
    uint32_t cfsr = scs_in32(0xd28);
    printk("BusFault cfsr=%08x bfar=%08x\n", (unsigned)cfsr, (unsigned)bfar);
    halt();
}

void usagefault_handler(void) {
    uint32_t cfsr = scs_in32(0xd28);
    printk("UsageFault cfsr=%08x\n", (unsigned)cfsr);
    halt();
}
