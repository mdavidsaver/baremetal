#include <stdint.h>
#include <stddef.h>

void load_os(void);

#define CCSRBASE 0xe1000000

#define PCI_CONF_IDX (CCSRBASE+0x8000)
#define PCI_CONF_DATA (CCSRBASE+0x8004)

#define PCI_CONF_ADDR(B,D,F,REG) \
 ((1<<31)|(((B)&0xff)<<16)|(((D)&0x1f)<<11)|(((F)&7)<<8)|((REG)&0xfc))

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

#define MAKEREG(N) \
static inline uint##N##_t in##N (uint32_t addr) { \
	uint##N##_t ret; volatile uint##N##_t *ptr = (void*)addr; \
	ret = *ptr;	__asm__ volatile ("eieio"::); return ret; } \
static inline void out##N (uint32_t addr, uint##N##_t val) { \
	volatile uint##N##_t *ptr = (void*)addr; *ptr = val; \
	__asm__ volatile ("eieio"::); }

MAKEREG(8)
MAKEREG(16)
MAKEREG(32)

#define MAKEPCI(N) \
static inline uint##N##_t pci_in##N(int b, int d, int f, uint16_t addr) { \
	out32(PCI_CONF_IDX, PCI_CONF_ADDR(b,d,f,addr)); \
	return le##N##toh(in##N(PCI_CONF_DATA)); } \
static inline void pci_out##N(int b, int d, int f, uint16_t addr, uint##N##_t val) { \
	out32(PCI_CONF_IDX, PCI_CONF_ADDR(b,d,f,addr)); \
	out##N(PCI_CONF_DATA, htole##N(val)); }

MAKEPCI(8)
MAKEPCI(16)
MAKEPCI(32)

static
uint32_t next_mmio = 0x80000000,
         next_io   = 0xe0000000;

static
void setup_host(void)
{
	/* Setup MMIO window 256 MB */
	out32(CCSRBASE+0x8c20, next_mmio>>12);
	out32(CCSRBASE+0x8c24, 0x00000000);
	out32(CCSRBASE+0x8c28, next_mmio>>12);
	out32(CCSRBASE+0x8c30, 0x8004401b);
	/* Setup IO window 16 MB */
	out32(CCSRBASE+0x8c40, next_io>>12);
	out32(CCSRBASE+0x8c44, 0x00000000);
	out32(CCSRBASE+0x8c48, next_io>>12);
	out32(CCSRBASE+0x8c50, 0x80088017);
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
	for(bar=0; bar<6; bar++) {
		config_bar(b,d,f,bar);
	}
	/* Set IRQ */
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

void Init(void)
{
	/* TODO: enable I2C bus */
	setup_host();
	walk_bus(0);
	load_os();
}
