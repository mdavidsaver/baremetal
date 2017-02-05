#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#include "ell.h"

typedef struct PCIRegion PCIRegion;
typedef struct PCIDevice PCIDevice;
typedef struct PCIBus PCIBus;

struct PCIRegion {
    unsigned inuse:1;
    unsigned isio:1;
    unsigned isbridge:1;
    uint32_t base, size;

    ELLNODE node; /* in PCIBus::mmio or PCIBus::io */
};

struct PCIDevice {
    uint8_t inuse;
    uint8_t B, D, F;
    uint8_t type;
    uint8_t hasmmio, hasio;

    PCIBus *parent, /* NULL for host bridge */
           *child; /* bridges only */

    ELLNODE node; /*in PCIBus::dev */

    PCIRegion bar[6];

    PCIRegion bridge_mmio, bridge_io;
};

typedef void (*pci_irq_alloc_fn)(PCIDevice*);

struct PCIBus {
    uint8_t inuse;
    uint8_t bus; /* our ID */
    uint8_t last_child; /* last assigned child bus.  bus==last_child -> no children */

    PCIDevice *parent; /* bridge device */
    ELLLIST dev, mmio, io;

    uint32_t mmio_next, io_next;
    
    pci_irq_alloc_fn irqfn;
    void *irqfn_data;
};

PCIBus *pci_alloc_host_bus(void);

int pci_scan_bus(PCIBus *bus);
void pci_show_bus(PCIBus *bus);

int pci_setup_bus(PCIBus *bus);

#endif /* PCI_H */
