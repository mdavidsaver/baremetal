#include <stdint.h>
#include <stddef.h>

void load_os(void);

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

static inline
uint16_t bswap16(uint16_t v)
{
	return (v<<8)|(v>>8);
}

static inline
uint32_t bswap32(uint32_t v)
{
	return (v<<24)|
		((v<<16)&0x00ff0000)|
		((v>>16)&0x0000ff00)|
		(v>>24);
}

#define le8toh(V) (V)
#define be8toh(V) (V)
#define le16toh(V) bswap16(V)
#define le32toh(V) bswap32(V)
#define be16toh(V) (V)
#define be32toh(V) (V)
#define htole8(V) (V)
#define htobe8(V) (V)
#define htole16(V) bswap16(V)
#define htole32(V) bswap32(V)
#define htobe16(V) (V)
#define htobe32(V) (V)

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

static
uint32_t next_mmio = 0x80000000,
         next_io   = 0xe0000000;

static
void setup_host(void)
{
	/* Setup MMIO window 256 MB */
	out32x(CCSRBASE, 0x8c20, next_mmio>>12);
	out32x(CCSRBASE, 0x8c24, 0x00000000);
	out32x(CCSRBASE, 0x8c28, next_mmio>>12);
	out32x(CCSRBASE, 0x8c30, 0x8004401b);
	/* Setup IO window 16 MB */
	out32x(CCSRBASE, 0x8c40, next_io>>12);
	out32x(CCSRBASE, 0x8c44, 0x00000000);
	out32x(CCSRBASE, 0x8c48, next_io>>12);
	out32x(CCSRBASE, 0x8c50, 0x80088017);
	/* Extra window used by tsi148, 256 MB */
	out32x(CCSRBASE, 0x8c60, 0xc0000000>>12);
	out32x(CCSRBASE, 0x8c64, 0x00000000);
	out32x(CCSRBASE, 0x8c68, 0xc0000000>>12);
	out32x(CCSRBASE, 0x8c70, 0x8004401b);
	/* TODO: setup inbound window(s) to support DMA */
}

static
uint8_t has_reg;

static
void config_bar(unsigned b, unsigned d, unsigned f, unsigned bar)
{
	uint8_t btype;
	uint16_t addr = 0x10+4*bar;
	uint32_t val, mask, size, base;

	val = pci_in32(b,d,f,addr);
	btype = val&1;
	if(btype) { /* IO space */
		mask = ~3;
		base = next_io;
	} else { /* memory */
		mask = ~0xf;
		base = next_mmio;
	}

	pci_out32(b,d,f,addr,mask);
	val = pci_in32(b,d,f,addr);

	val&=mask;

	size = val & ~(val-1); /* find LSB */
	if(size==0)
		return;

	/* align base address to a mulitple of size */
	if(base&(size-1)) {
		base = (base|(size-1))+1;
	}

	pci_out32(b,d,f,addr,base);
	if(btype) {
		next_io = base + size;
		has_reg |= 1;
	} else {
		next_mmio = base + size;
		has_reg |= 2;
	}
}

static
void config_device(unsigned b, unsigned d, unsigned f)
{
	uint32_t ena;
	unsigned bar;
	has_reg = 0;
	uint8_t irqpin;
	for(bar=0; bar<6; bar++) {
		config_bar(b,d,f,bar);
	}
	/* Set IRQ lines
	 *  A mvme3100 has (at least) three PCI busses
	 * Bus 0 (A) has only two devices w/ IRQ, and an arbitrary IRQ assignment
	 * slot 17 (tsi148)
	 *   IRQ0, IRQ1, IRQ2, IRQ3
	 * slot 20 (sata0
	 *   IRQ2
	 * Bus 1 (B) and 2 (C) have the conventional rotatin assignments
	 *  Bus 1
	 *    slot 0 IRQ4, IRQ5, IRQ6, IRQ7
	 *    slot 1 IRQ5, IRQ6, IRQ7, IRQ4
	 *  Bus 2 slot 0
	 *   IRQ4, IRQ5, IRQ6
	 *   ...
	 */
	irqpin = pci_in8(b,d,f,0x3d);
	if(b==0 && d==0x11 && f==0 && irqpin) {
		pci_out8(b,d,f,0x3c, 0+irqpin-1);
	} else if(b==0 && d==0x14 && f==0 && irqpin) {
		pci_out8(b,d,f,0x3c, (2+irqpin-1)&3);
	} else if(irqpin) {
		pci_out8(b,d,f,0x3c, (4+d+irqpin-1)&3);
	}
	/* Enable */
	ena = has_reg | 0x0140;
	pci_out16(b,d,f, 4, ena);
}

static
void walk_bus(unsigned b)
{
	unsigned d;
	for(d=0; d<0x20; d++) {
		uint16_t val = pci_in16(b,d,0, 0);
		if(val==0xffff)
			continue;
		config_device(b,d,0);
	}
}

typedef struct {
	unsigned vendor, device, subvendor, subdevice, devclass;
} pciid;

static
int pci_find(const pciid *id, unsigned index, unsigned *rb, unsigned *rd, unsigned *rf)
{
	unsigned d;
	for(d=0; d<0x20; d++) {
		uint32_t val = pci_in16(0,d,0,0);
		if(val==0xffff)
			continue;
		if(d!=0x11)
			continue;
		if(id->vendor!=(unsigned)-1 && val!=id->vendor)
			continue;
		val = pci_in16(0,d,0,2);
		if(id->device!=(unsigned)-1 && val!=id->device)
			continue;
		val = pci_in16(0,d,0,0x2c);
		if(id->subvendor!=(unsigned)-1 && val!=id->subvendor)
			continue;
		val = pci_in16(0,d,0,0x2e);
		if(id->subdevice!=(unsigned)-1 && val!=id->subdevice)
			continue;
		if(index!=0) {
			index--;
			continue;
		}
		*rb = 0;
		*rd = d;
		*rf = 0;
		return 0;
	}
	return -1;
}

static
void prepare_tsi148(void)
{
	uint32_t bar;
	pciid tsi148 = {
		.vendor = 0x10e3,
		.device = 0x0148,
		.subvendor = -1,
		.subdevice = -1,
		.devclass = -1,
	};
	unsigned b, d, f;
	if(pci_find(&tsi148, 0, &b,&d,&f))
		return;
	bar = pci_in32(b,d,f,0x10)&0xffffff00;
	if(!bar)
		return;
	out32x(bar, 0x604, 1<<27); /* master enable on */
}

void Init(void)
{
	uint16_t vend = pci_in16(0,0,0,0),
	         device = pci_in16(0,0,0,2);
	if(vend!=0x1957 || device!=0x30)
		return; /* wrong host bridge, something is really wrong here */

	/* TODO: enable I2C bus */
	setup_host();
	walk_bus(0);
	prepare_tsi148();
	load_os();
}
