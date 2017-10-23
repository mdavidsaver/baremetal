#ifndef PCI2_H
#define PCI2_H

#include <stdint.h>

struct pci_info;

typedef void (*pci_irq_alloc_fn)(unsigned b, unsigned d, unsigned f, struct pci_info *info);
int pci_setup(struct pci_info *info);

typedef struct pci_info{
    uint32_t next_mmio;
    uint32_t next_io;
    uint8_t next_bus;
    pci_irq_alloc_fn irqfn;
} pci_info;

#endif /* PCI2_H */
