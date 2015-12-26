#ifndef MMIO_H
#define MMIO_H
#include <stdint.h>

#define CCSRBASE 0xe1000000

#define PCI_CONF_IDX (CCSRBASE+0x8000)
#define PCI_CONF_DATA (CCSRBASE+0x8004)

/* 0x80000000 - enable
 * 0x00ff0000 - bus
 * 0x0000f800 - device
 * 0x00000700 - function
 * 0x000000fc - address
 * 0x00000003 - data offset
 */
#define PCI_CONF_ADDR(B,D,F,REG) \
 ((1<<31)|(((B)&0xff)<<16)|(((D)&0x1f)<<11)|(((F)&7)<<8)|((REG)&0xff))


static inline uint8_t in8x(uint32_t base, uint32_t off) {
	uint8_t val;
	__asm__ ("eieio\n\t lbzx %0, %1, %2" : "=r"(val) : "b"(base), "r"(off) : "memory");
	return val;
}
static inline uint16_t in16x(uint32_t base, uint32_t off) {
	uint16_t val;
	__asm__ ("eieio\n\t lhzx %0, %1, %2" : "=r"(val) : "b"(base), "r"(off) : "memory");
	return val;
}
static inline uint32_t in32x(uint32_t base, uint32_t off) {
	uint32_t val;
	__asm__ ("eieio\n\t lwzx %0, %1, %2" : "=r"(val) : "b"(base), "r"(off) : "memory");
	return val;
}

static inline void out8x(uint32_t base, uint32_t off, uint8_t val) {
	__asm__("stbx %0, %1, %2\n\t eieio" :: "r"(val), "b"(base), "r"(off) : "memory");
}
static inline void out16x(uint32_t base, uint32_t off, uint16_t val) {
	__asm__("sthx %0, %1, %2\n\t eieio" :: "r"(val), "b"(base), "r"(off) : "memory");
}
static inline void out32x(uint32_t base, uint32_t off, uint32_t val) {
	__asm__("stwx %0, %1, %2\n\t eieio" :: "r"(val), "b"(base), "r"(off) : "memory");
}

static inline uint8_t pci_in8x(uint32_t paddr) {
	uint32_t base = PCI_CONF_IDX;
	out32x(base, 0, paddr&~3);
	return in8x(base, 4+(paddr&3));
}
static inline uint16_t pci_in16x(uint32_t paddr) {
	uint32_t base = PCI_CONF_IDX;
	uint16_t val;
	out32x(base, 0, paddr&~3);
	__asm__ ("eieio\n\t lhbrx %0, %1, %2" : "=r"(val) : "b"(base), "r"(4+(paddr&3)) : "memory");
	return val;
}
static inline uint32_t pci_in32x(uint32_t paddr) {
	uint32_t base = PCI_CONF_IDX;
	uint32_t val;
	out32x(base, 0, paddr);
	__asm__ ("eieio\n\t lwbrx %0, %1, %2" : "=r"(val) : "b"(base), "r"(4) : "memory");
	return val;
}
#define pci_in8(B,D,F,ADDR) pci_in8x(PCI_CONF_ADDR(B,D,F,ADDR))
#define pci_in16(B,D,F,ADDR) pci_in16x(PCI_CONF_ADDR(B,D,F,ADDR))
#define pci_in32(B,D,F,ADDR) pci_in32x(PCI_CONF_ADDR(B,D,F,ADDR))

static inline void pci_out8x(uint32_t paddr, uint8_t val) {
	uint32_t base = PCI_CONF_IDX;
	out32x(base, 0, paddr&~3);
	out8x(base, 4+(paddr&3), val);
}
static inline void pci_out16x(uint32_t paddr, uint16_t val) {
	uint32_t base = PCI_CONF_IDX;
	out32x(base, 0, paddr&~3);
	__asm__("sthbrx %0, %1, %2\n\t eieio" :: "r"(val), "b"(base), "r"(4+(paddr&3)) : "memory");
}
static inline void pci_out32x(uint32_t paddr, uint32_t val) {
	uint32_t base = PCI_CONF_IDX;
	out32x(base, 0, paddr&~3);
	__asm__("stwbrx %0, %1, %2\n\t eieio" :: "r"(val), "b"(base), "r"(4) : "memory");
}
#define pci_out8(B,D,F,ADDR,VAL) pci_out8x(PCI_CONF_ADDR(B,D,F,ADDR),VAL)
#define pci_out16(B,D,F,ADDR,VAL) pci_out16x(PCI_CONF_ADDR(B,D,F,ADDR),VAL)
#define pci_out32(B,D,F,ADDR,VAL) pci_out32x(PCI_CONF_ADDR(B,D,F,ADDR),VAL)

#define le8toh(V) (V)
#define be8toh(V) (V)
#define le16toh(V) __builtin_swap16(V)
#define le32toh(V) __builtin_swap32(V)
#define be16toh(V) (V)
#define be32toh(V) (V)
#define htole8(V) (V)
#define htobe8(V) (V)
#define htole16(V) __builtin_swap16(V)
#define htole32(V) __builtin_swap32(V)
#define htobe16(V) (V)
#define htobe32(V) (V)

#endif /* MMIO_H */
