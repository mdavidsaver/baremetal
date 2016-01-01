
#include "common.h"
#include "cpu.h"

void hardfault_handler(void) {
    printk("HardFault\n");
    halt();
}

void memfault_handler(void) {
    uint32_t cfsr = scs_in32(0xd28);
    printk("MemFault cfsr=%08x\n", (unsigned)cfsr);
    halt();
}

void busfault_handler(void) {
    uint32_t cfsr = scs_in32(0xd28);
    printk("BusFault cfsr=%08x\n", (unsigned)cfsr);
    halt();
}

void usagefault_handler(void) {
    uint32_t cfsr = scs_in32(0xd28);
    printk("UsageFault cfsr=%08x\n", (unsigned)cfsr);
    halt();
}
