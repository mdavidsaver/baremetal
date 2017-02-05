
#include "stddef.h"

#include "common.h"
#include "mmio.h"
#include "pci.h"
#include "pci_def.h"

#define PCI_MAX_DEVICES 16
#define PCI_MAX_BUSES 4

static PCIDevice pci_device_pool[PCI_MAX_DEVICES];

static PCIBus pci_bus_pool[PCI_MAX_BUSES];

static
PCIDevice *pci_alloc_device(void)
{
    unsigned i;
    for(i=0; i<PCI_MAX_DEVICES; i++) {
        PCIDevice *dev = &pci_device_pool[i];
        if(!dev->inuse) {
            memset(dev, 0, sizeof(*dev));
            dev->inuse = 1;
            return dev;
        }
    }
    printk("ERROR: PCI device descriptors exhausted\n");
    return NULL;
}

static
PCIBus    *pci_alloc_bus(void)
{
    unsigned i;
    for(i=0; i<PCI_MAX_BUSES; i++) {
        PCIBus *bus = &pci_bus_pool[i];
        if(!bus->inuse) {
            memset(bus, 0, sizeof(*bus));
            bus->inuse = 1;

            return bus;
        }
    }
    printk("ERROR: PCI bus descriptors exhausted\n");
    return NULL;
}

PCIBus *pci_alloc_host_bus(void)
{
    PCIBus *bus = pci_alloc_bus();
    if (bus) {
    }
    return bus;
}

/* add memory region to sorted list of a Bus */
static
void pci_bus_append_region(PCIBus *bus, PCIRegion *region)
{
    ELLLIST *list = region->isio ? &bus->io : &bus->mmio;
    ELLNODE *cur;

    if(!region->inuse) return;
    
    /* insert in sorted order, decending (largest first) */
    for(cur = ellFirst(list); cur; cur = ellNext(cur)) {
        PCIRegion *C = container(cur, PCIRegion, node);
        if(region->size > C->size)
            break;
    }

    ellInsertBefore(list, cur, &region->node);
}

static
int pci_scan_bar(PCIDevice *dev, unsigned n)
{
    PCIRegion *bar = &dev->bar[n];
    uint32_t val, mask;

    printk("  Probe BAR %x\n", n);
    
    val = pci_in32(dev->B,dev->D,dev->F, PCI_BAR(n));

    bar->isio = !!(val&PCI_BAR_SPACE_MASK);

    if(!bar->isio && (val&PCI_BAR_MEM_TYPE_MASK)==PCI_BAR_MEM_TYPE_64) {
        printk("ERROR: 64-bit BAR not supported\n");
        return -1; /* we don't know how to configure 64-bit BARs */
    } else if(!bar->isio && (val&PCI_BAR_MEM_TYPE_MASK)!=PCI_BAR_MEM_TYPE_32) {
        printk("WARN: ignore non MMIO32\n");
        return 0;  /* ignore <1MB or prefetchable */
    }

    if(bar->isio) mask = PCI_BAR_IO_MASK;
    else          mask = PCI_BAR_MEM_MASK;

    pci_out32(dev->B,dev->D,dev->F, PCI_BAR(n), mask);
    val = pci_in32(dev->B,dev->D,dev->F,PCI_BAR(n));
    val&=mask;
    bar->size = val & ~(val-1); /* find LSB */

    if(bar->size>0) {
        bar->inuse = 1;
        pci_bus_append_region(dev->parent, bar);
        printk("  BAR %x size=%x\n", n, (unsigned)bar->size);
    }

    return 0;
}

static
void pci_scan_device(PCIDevice *dev)
{
    unsigned n;

    printk("Probe regular device\n");
    
    for(n=0; n<6; n++) {
        if(pci_scan_bar(dev, n)) {
            n++; // 64-bit bar takes 2x
            continue;
        }
    }
}

/* find bounding box of bridge regions */
static
void pci_bridge_region(PCIRegion *bridge, ELLLIST *regions)
{
    ELLNODE *cur;
    int8_t first = 1;
    uint32_t base = 0, limit = 0,
             mask = bridge->isio ? 0xfff : 0xfffff;

    foreach_ell(cur, regions) {
        PCIRegion *reg = container(cur, PCIRegion, node);
        if(first || reg->base<base) {
            base = reg->base;
            printk("  NEW BASE %x\n", (unsigned)base);
        }
        if(first || (reg->base+reg->size)>limit) {
            limit = reg->base+reg->size;
            printk("  NEW LIMIT %x\n", (unsigned)limit);
        }
        first = 0;    
    }

    bridge->isbridge = 1;
    bridge->inuse = base<limit;
    /* base already aligned in pci_regions_offset() */
    bridge->base = base;
    bridge->size = ((limit-base-1u)|mask)+1u;
    
    printk("coelese bridge %s %x %x\n", bridge->isio ? "IO" : "MMIO",
           (unsigned)bridge->base, (unsigned)bridge->size);
}

static
void pci_scan_bridge(PCIDevice *dev)
{
    PCIBus *bus;
    unsigned n;

    printk("Probe bridge device\n");

    for(n=0; n<2; n++) {
        if(pci_scan_bar(dev, n)) {
            n++; // 64-bit bar takes 2x
            continue;
        }
    }
    
    if(dev->B==0 && dev->D==0 && dev->F==0)
        return; // host bridge

    printk  ("Probe child bus\n");

    bus = pci_alloc_bus();
    if(!bus)
        return;
    bus->parent = dev;
    dev->child = bus;

    bus->last_child = bus->bus = ++dev->parent->last_child;

    bus->mmio_next = ((dev->parent->mmio_next-1u)|0xfffffu)+1u;
    bus->io_next = ((dev->parent->io_next-1u)|0xfffu)+1u;

    pci_out8(dev->B, dev->D, dev->F, PCI_PRIMARY_BUS, dev->parent->bus);
    pci_out8(dev->B, dev->D, dev->F, PCI_SECONDARY_BUS, bus->bus);
    pci_out8(dev->B, dev->D, dev->F, PCI_SUBORDINATE_BUS, 0xff);

    if(pci_scan_bus(bus)) {
        dev->child = NULL;
        bus->inuse = 0;
        return;
    }

    bus->irqfn = dev->parent->irqfn;
    bus->irqfn_data = dev->parent->irqfn_data;

    dev->parent->last_child = bus->last_child;

    dev->parent->mmio_next = bus->mmio_next;
    dev->parent->io_next = bus->io_next;

    pci_out8(dev->B, dev->D, dev->F, PCI_SUBORDINATE_BUS, bus->last_child);

    printk("  Child bus  primary=%x secondary=%x subordinate=%x\n",
           dev->parent->bus, bus->bus, bus->last_child);

    if(!ellEmpty(&bus->mmio)) {
        pci_bridge_region(&dev->bridge_mmio, &bus->mmio);
    }
    if(!ellEmpty(&bus->io)) {
        dev->bridge_io.inuse = 1;
        pci_bridge_region(&dev->bridge_io, &bus->io);
    }
}

/* assign offsets to regions in list taking alignment into consideration
 */
static
void pci_regions_offset(uint32_t *start, ELLLIST *list)
{
    ELLNODE *cur;
    uint32_t offset = *start;
    printk("Assign addresses from %x\n", (unsigned)offset);
    foreach_ell(cur, list) {
        PCIRegion *reg = container(cur, PCIRegion, node);
        uint32_t mask = 0u;

        if(!reg->isbridge) {
            mask = reg->size-1u;

        } else {
            if(reg->base+reg->size > offset) {
                printk("ERROR: bridge and BARs would overlap!\n");
                break;
            }
            /* bridges already assigned */
            continue;
        }

        /* align offset to next multiple of region size (may not changed) */
        offset = ((offset-1u)|mask)+1u;

        reg->base = offset;
        offset += reg->size;
        printk(" %x %x\n", (unsigned)reg->base, (unsigned)reg->size);
    }
    *start = offset;
}

/* Accumulate info on Devices and configure Bridges
 *
 * 1. scan all device on bus and allocate PCIDevice
 * 2. probe BARs and fill in PCIRegion::size
 * 3. Recurse into bridges
 * 4. sort region list
 * 5. fill in PCIBus::mmio_size and PCIBus::io_size
 */
int pci_scan_bus(PCIBus *bus)
{
    unsigned b = bus->bus, d, f = 0;
    printk("Scan Bus %x\n", b);
    for(d=0; d<0x20; d++) {
        PCIDevice *dev;
        uint32_t cmd;

        uint16_t val = pci_in16(b,d,f, PCI16_VENDOR_ID);
        if(val==0xffff)
            continue;
        printk("Found Device %x:%x.%x\n", b, d, f);

        /* disable response to BARs and revoke bus master */
        cmd = pci_in32(b,d,f, PCI_COMMAND);
        cmd &= ~(PCI_COMMAND_IO|PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER);
        pci_out32(b,d,f, PCI_COMMAND, cmd);

        dev = pci_alloc_device();
        if (!dev)
            break;
        dev->B = b; dev->D = d; dev->F = f;
        dev->parent = bus;

        dev->type = pci_in8(b,d,f, PCI8_HEADER_TYPE);
        switch(dev->type) {
        case PCI8_HEADER_TYPE_NORMAL:
            pci_scan_device(dev);
            break;
        case PCI8_HEADER_TYPE_BRIDGE:
            pci_scan_bridge(dev);
            break;
        default:
            dev->inuse = 0;
            continue;
        }

        pci_bus_append_region(bus, &dev->bridge_mmio);
        pci_bus_append_region(bus, &dev->bridge_io);

        ellPushBack(&bus->dev, &dev->node);
    }

    pci_regions_offset(&bus->mmio_next, &bus->mmio);
    pci_regions_offset(&bus->io_next, &bus->io);

    return 0;
}

/* Does PCI config writes
 * 1. Assign addresses
 * 2. Assign IRQs
 * 3. Enable devices
 * 4. Recurse into bridges
 */
int pci_setup_bus(PCIBus *bus)
{
    ELLNODE *cur;
    foreach_ell(cur, &bus->dev) {
        PCIDevice *dev = container(cur, PCIDevice, node);
        unsigned i, nbars = dev->type==PCI8_HEADER_TYPE_NORMAL ? 6 : 2;
        uint32_t cmd;
        uint8_t hasmem = 0, hasio = 0;

        for(i=0; i<nbars; i++) {
            PCIRegion *bar = &dev->bar[i];
            uint32_t val, mask = bar->isio ? PCI_BAR_IO_MASK : PCI_BAR_MEM_MASK;
            if(!bar->inuse) continue;
            val = pci_in32(dev->B, dev->D, dev->F, PCI_BAR(i));
            val &= ~mask;
            val |= bar->base&mask;
            hasmem |= !bar->isio;
            hasio  |= !!bar->isio;
            pci_out32(dev->B, dev->D, dev->F, PCI_BAR(i), val);
            printk("Set %x:%x.%x BAR %x %x\n", dev->B, dev->D, dev->F, i,
                   (unsigned)val);
        }

        if(bus->irqfn)
            (*bus->irqfn)(dev);

        if(dev->child) {
            if(dev->bridge_mmio.inuse) {
                uint32_t base = dev->bridge_mmio.base,
                         limit = base + dev->bridge_mmio.size - 1;
                pci_out16(dev->B, dev->D, dev->F, PCI_MEMORY_BASE, base>>16);
                pci_out16(dev->B, dev->D, dev->F, PCI_MEMORY_LIMIT,limit>>16);

                printk("BRIDGE %x:%x.%x MMIO %x %x\n",  dev->B, dev->D, dev->F,
                       base, limit);
                
                hasmem = 1;
            } else {
                /* limit < base disables */
                pci_out16(dev->B, dev->D, dev->F, PCI_MEMORY_BASE, 0x0010);
                pci_out16(dev->B, dev->D, dev->F, PCI_MEMORY_LIMIT,0x0000);
                printk("BRIDGE %x:%x.%x MMIO disable\n",  dev->B, dev->D, dev->F);
            }
            
            if(dev->bridge_io.inuse) {
                uint16_t base = dev->bridge_io.base,
                         limit = base + dev->bridge_io.size - 1;
                pci_out8(dev->B, dev->D, dev->F, PCI_IO_BASE, base>>8u);
                pci_out8(dev->B, dev->D, dev->F, PCI_IO_LIMIT,limit>>8u);

                printk("BRIDGE %x:%x.%x IO %x %x\n",  dev->B, dev->D, dev->F,
                       base, limit);
                hasio = 1;
            } else {
                /* limit < base disables */
                pci_out8(dev->B, dev->D, dev->F, PCI_IO_BASE, 0x10);
                pci_out8(dev->B, dev->D, dev->F, PCI_IO_LIMIT,0x00);
                printk("BRIDGE %x:%x.%x IO disable\n",  dev->B, dev->D, dev->F);
            }

            pci_out16(dev->B, dev->D, dev->F, PCI_IO_BASE_UPPER16, 0);
            pci_out16(dev->B, dev->D, dev->F, PCI_IO_LIMIT_UPPER16,0);
            
            if(pci_setup_bus(dev->child)) {
                printk("Error in setup of bus %x\n", dev->child->bus);
                continue;
            }
        }

        /* enable response to BARs and bridges */
        cmd = pci_in32(dev->B, dev->D, dev->F, PCI_COMMAND);
        if(hasmem) cmd |= PCI_COMMAND_MEMORY;
        if(hasio)  cmd |= PCI_COMMAND_IO;
        pci_out32(dev->B, dev->D, dev->F, PCI_COMMAND, cmd);
    }
    
    return 0;
}


static
void pci_show_regions(ELLLIST *list)
{
    ELLNODE *cur;
    foreach_ell(cur, list) {
        PCIRegion *reg = container(cur, PCIRegion, node);
        printk(" Region offset=%x size=%x\n",
                   (unsigned)reg->base,
                   (unsigned)reg->size);
    }
}

void pci_show_bus(PCIBus *bus)
{
    ELLNODE *cur;

    printk("Bus %x\n", bus->bus);
    foreach_ell(cur, &bus->dev) {
        PCIDevice *dev = container(cur, PCIDevice, node);
        unsigned n;

        printk("Device %x:%x.%x\n", dev->B, dev->D, dev->F);
        for(n=0; n<6; n++) {
            PCIRegion *bar = &dev->bar[n];
            if(!bar->inuse) continue;
            printk("  %s BAR%x offset=%x size=%x\n",
                   bar->isio ? "IO" : "MMIO",
                   n,
                   (unsigned)bar->base,
                   (unsigned)bar->size);
        }
        
        if(dev->bridge_mmio.inuse) {
            PCIRegion *bar = &dev->bridge_mmio;
            if(!bar->inuse) continue;
            printk("  BRIDGE MEM offset=%x size=%x\n",
                   (unsigned)bar->base,
                   (unsigned)bar->size);
            
        }
        if(dev->bridge_io.inuse) {
            PCIRegion *bar = &dev->bridge_io;
            if(!bar->inuse) continue;
            printk("  BRIDGE MEM offset=%x size=%x\n",
                   (unsigned)bar->base,
                   (unsigned)bar->size);
            
        }
    }

    printk("MMIO Regions\n");
    pci_show_regions(&bus->mmio);
    printk("IO Regions\n");
    pci_show_regions(&bus->io);

    foreach_ell(cur, &bus->dev) {
        PCIDevice *dev = container(cur, PCIDevice, node);
        if(!dev->child) continue;
        pci_show_bus(dev->child);
    }
}

