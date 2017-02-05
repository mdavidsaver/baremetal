#ifndef PCI_DEFS_H
#define PCI_DEFS_H

#define PCI16_VENDOR_ID 0x00
#define PCI16_DEVICE_ID 0x02

#define PCI_COMMAND     0x04    /* 16 bits */
#define  PCI_COMMAND_IO     0x1 /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */


#define PCI8_HEADER_TYPE 0x0e
#define  PCI8_HEADER_TYPE_NORMAL     0
#define  PCI8_HEADER_TYPE_BRIDGE     1
#define  PCI8_HEADER_TYPE_CARDBUS    2

#define PCI_BAR(N) (0x10 + 4*(N))

#define  PCI_BAR_SPACE_MASK     0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BAR_SPACE_IO  0x01
#define  PCI_BAR_SPACE_MEMORY  0x00
#define  PCI_BAR_MEM_TYPE_MASK 0x06
#define  PCI_BAR_MEM_TYPE_32   0x00    /* 32 bit address */
#define  PCI_BAR_MEM_TYPE_1M   0x02    /* Below 1M */
#define  PCI_BAR_MEM_TYPE_64   0x04    /* 64 bit address */
#define  PCI_BAR_MEM_PREFETCH  0x08    /* prefetchable */
#define  PCI_BAR_MEM_MASK  (~0x0fUL)
#define  PCI_BAR_IO_MASK   (~0x03UL)

#define PCI_INTERRUPT_LINE  0x3c    /* 8 bits */
#define PCI_INTERRUPT_PIN   0x3d    /* 8 bits */


/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS     0x18    /* Primary bus number */
#define PCI_SECONDARY_BUS   0x19    /* Secondary bus number */
#define PCI_SUBORDINATE_BUS 0x1a    /* Highest bus number behind the bridge */

#define PCI_IO_BASE     0x1c    /* I/O range behind the bridge */
#define PCI_IO_LIMIT        0x1d
#define  PCI_IO_RANGE_TYPE_MASK 0x0fUL  /* I/O bridging type */
#define  PCI_IO_RANGE_TYPE_16   0x00
#define  PCI_IO_RANGE_TYPE_32   0x01
#define  PCI_IO_RANGE_MASK  (~0x0fUL) /* Standard 4K I/O windows */
#define  PCI_IO_1K_RANGE_MASK   (~0x03UL) /* Intel 1K I/O windows */
#define PCI_SEC_STATUS      0x1e    /* Secondary status register, only bit 14 used */
#define PCI_MEMORY_BASE     0x20    /* Memory range behind */
#define PCI_MEMORY_LIMIT    0x22
#define  PCI_MEMORY_RANGE_TYPE_MASK 0x0fUL
#define  PCI_MEMORY_RANGE_MASK  (~0x0fUL)

#define PCI_IO_BASE_UPPER16 0x30    /* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16    0x32


#endif /* PCI_DEFS_H */
