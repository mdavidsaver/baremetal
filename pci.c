/* Minimalist enumeration of PCI
 * recursively scans PCI-PCI bridges
 *
 * does not handle multi-function devices
 * does not optimize for efficient use of address space
 * does not handle prefetchable memory
 */


#include "stddef.h"

#include "common.h"
#include "mmio.h"
#include "pci_def.h"
#include "pci.h"

static
int pci_setup_bar(unsigned b, unsigned d, unsigned f, unsigned bar, pci_info *info)
{
    uint32_t val, mask, size;
    
    val = pci_in32(b,d,f, PCI_BAR(bar));

    mask = (val&PCI_BAR_SPACE_MASK) ? PCI_BAR_IO_MASK : PCI_BAR_MEM_MASK;

    if(mask==PCI_BAR_MEM_MASK && (val&PCI_BAR_MEM_TYPE_MASK)==PCI_BAR_MEM_TYPE_64) {
        printk("  BAR%u ERROR: 64-bit BAR not supported\n", bar);
        return -1; /* we don't know how to configure 64-bit BARs */
    } else if(mask==PCI_BAR_MEM_MASK && (val&PCI_BAR_MEM_TYPE_MASK)!=PCI_BAR_MEM_TYPE_32) {
        printk("  BAR%u WARN: ignore non MMIO32\n", bar);
        return 0;  /* ignore <1MB or prefetchable */
    };

    pci_out32(b,d,f, PCI_BAR(bar), mask);
    val = pci_in32(b,d,f,PCI_BAR(bar));
    val&=mask;
    size = val & ~(val-1); /* find LSB */

    if(size==0) return 0;

    if(mask==PCI_BAR_MEM_MASK) {
        val =  info->next_mmio;
    } else {
        val =  info->next_io;
    }
    /* round up to multiple of size (power of 2) */
    val = ((val-1)|(size-1u))+1;

    pci_out32(b,d,f, PCI_BAR(bar), val);
    printk("  BAR%u %sio %08x -> %08x\n",
        bar, (mask==PCI_BAR_MEM_MASK) ? "mm" : "",
        (unsigned)val, (unsigned)(val+size-1u)
    );

    val += size;

    if(mask==PCI_BAR_MEM_MASK) {
        info->next_mmio = val;
    } else {
        info->next_io = val;
    }
    return 0;
}

static
int pci_setup_device(unsigned b, unsigned d, unsigned f, pci_info *info)
{
    unsigned bar, cmd;
    pci_info atstart = *info;

    printk("Found Device %x:%x.%x\n", b, d, f);

    for(bar=0; bar<6; bar++) {
        if(pci_setup_bar(b, d, f, bar, info))
            bar++; /* skip 64-bit bar */
    }
    info->irqfn(b,d,f,info);

    cmd = pci_in32(b,d,f, PCI_COMMAND);
    if(atstart.next_mmio!=info->next_mmio) {
        cmd |= PCI_COMMAND_MEMORY;
    }
    if(atstart.next_io!=info->next_io) {
        cmd |= PCI_COMMAND_IO;
    }
    pci_out32(b,d,f, PCI_COMMAND, cmd);
    
    return 0;
}

static
int pci_setup_bridge(unsigned b, unsigned d, unsigned f, pci_info *info)
{
    uint32_t cmd;
    pci_info atstart;

    if(b==0 && d==0 && f==0) return 0; /* skip host bridge */
    
    printk("Found Bridge %x:%x.%x\n", b, d, f);

    /* we don't map bridge BARs or interrupts */

    pci_out8(b,d,f, PCI_PRIMARY_BUS, b);
    pci_out8(b,d,f, PCI_SECONDARY_BUS, b+1);
    pci_out8(b,d,f, PCI_SUBORDINATE_BUS, 0xff);

    /* ensure that addresses are aligned for bridge base */
    info->next_mmio = ((info->next_mmio-1)|0xffff)+1;
    info->next_io = ((info->next_io-1)|0xff)+1;

    atstart = *info;

    if(pci_setup(info))
        return 1;

    pci_out8(b,d,f, PCI_SUBORDINATE_BUS, info->next_bus-1u);

    printk("Bridge Child bus  primary=%u secondary=%u subordinate=%u\n",
           b, b+1, info->next_bus-1u);

    /* ensure that addresses are aligned for bridge limit */
    info->next_mmio = ((info->next_mmio-1)|0xffff)+1;
    info->next_io = ((info->next_io-1)|0xff)+1;

    cmd = pci_in32(b,d,f, PCI_COMMAND);
    if(atstart.next_mmio!=info->next_mmio) {
        cmd |= PCI_COMMAND_MEMORY;
        uint32_t base  = atstart.next_mmio;
        uint32_t limit = info->next_mmio-1;
        pci_out16(b,d,f, PCI_MEMORY_BASE, base>>16);
        pci_out16(b,d,f, PCI_MEMORY_LIMIT,limit>>16);

        printk("BRIDGE %x:%x.%x MMIO %08x -> %08x\n",  b,d,f, (unsigned)base, (unsigned)limit);
    } else {
        /* limit < base disables */
        pci_out16(b,d,f, PCI_MEMORY_BASE, 0x0010);
        pci_out16(b,d,f, PCI_MEMORY_LIMIT,0x0000);
        printk("BRIDGE %x:%x.%x MMIO disable\n",  b,d,f);
    }
    if(atstart.next_io!=info->next_io) {
        cmd |= PCI_COMMAND_IO;
        uint32_t base  = atstart.next_io;
        uint32_t limit = info->next_io-1;
        pci_out16(b,d,f, PCI_IO_BASE, base>>8);
        pci_out16(b,d,f, PCI_IO_LIMIT,limit>>8);

        printk("BRIDGE %x:%x.%x IO %08x -> %08x\n",  b,d,f, (unsigned)base, (unsigned)limit);
    } else {
        /* limit < base disables */
        pci_out16(b,d,f, PCI_IO_BASE, 0x0010);
        pci_out16(b,d,f, PCI_IO_LIMIT,0x0000);
        printk("BRIDGE %x:%x.%x IO disable\n",  b,d,f);
    }
    pci_out32(b,d,f, PCI_COMMAND, cmd);
    
    
    return 0;
}

int pci_setup(pci_info *info)
{
    unsigned b = info->next_bus++;
    unsigned d;
    const unsigned f=0;

    printk("Scan Bus %u\n", b);
    for(d=0; d<0x20; d++) {
        uint32_t cmd;
        uint16_t val = pci_in16(b,d,f, PCI16_VENDOR_ID);
        if(val==0xffff)
            continue;

        /* disable response to BARs and revoke bus master */
        cmd = pci_in32(b,d,f, PCI_COMMAND);
        cmd &= ~(PCI_COMMAND_IO|PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER);
        pci_out32(b,d,f, PCI_COMMAND, cmd);

        switch(pci_in8(b,d,f, PCI8_HEADER_TYPE)) {
        case PCI8_HEADER_TYPE_NORMAL:
            if(pci_setup_device(b,d,f,info))
                return 1;
            break;
        case PCI8_HEADER_TYPE_BRIDGE:
            if(pci_setup_bridge(b,d,f,info))
                return 1;
            break;
        default:
            printk("Ignore Device %x:%x.%x\n", b, d, f);
            continue;
        }
    }
    return 0;
}
