#include <stdint.h>
#include <stddef.h>

#include "mmio.h"
#include "tlb.h"

#define NELEMENTS(ARR) (sizeof(ARR)/sizeof(ARR[0]))

void load_os(uint32_t); /* in init-tom.S */

static const
tlbentry initial_mappings[] = {
	/* leave initial ROM and RAM mappings in place (first 3) */

	/* CCSR (after relocation) */
	{.mas0 = MAS0_TLB1,
	 .mas1 = MAS1_V|MAS1_TSIZE(0x5), /* 1 MB */
	 .mas2 = MAS2_EPN(0xe1000000)|MAS2_DEVICE,
	 .mas3 = MAS3_RPN(0xe1000000)|MAS3_DEVICE,
	},

	/* mvme3100 CPLD registers */
	{.mas0 = MAS0_TLB1,
	 .mas1 = MAS1_V|MAS1_TSIZE(0x7), /* 16 MB */
	 .mas2 = MAS2_EPN(0xe2000000)|MAS2_DEVICE,
	 .mas3 = MAS3_RPN(0xe2000000)|MAS3_DEVICE,
	},

	/* PCI IO ports */
	{.mas0 = MAS0_TLB1,
	 .mas1 = MAS1_V|MAS1_TSIZE(0x7), /* 16 MB */
	 .mas2 = MAS2_EPN(0xe0000000)|MAS2_DEVICE,
	 .mas3 = MAS3_RPN(0xe0000000)|MAS3_DEVICE,
	},

	/* PCI (cf. setup_host()) */
	{.mas0 = MAS0_TLB1,
	 .mas1 = MAS1_V|MAS1_TSIZE(0x9), /* 256 MB */
	 .mas2 = MAS2_EPN(0x80000000)|MAS2_DEVICE,
	 .mas3 = MAS3_RPN(0x80000000)|MAS3_DEVICE,
	},

	/* PCI (cf. setup_host()) */
	{.mas0 = MAS0_TLB1,
	 .mas1 = MAS1_V|MAS1_TSIZE(0x9), /* 256 MB */
	 .mas2 = MAS2_EPN(0xc0000000)|MAS2_DEVICE,
	 .mas3 = MAS3_RPN(0xc0000000)|MAS3_DEVICE,
	},
};

static
void setup_tlb(void)
{
	tlbentry disabled;
	unsigned idx, n;
	/* Leave first 3 mappings (ROM and RAM) in place,
	 * copy in remaining
	 */
	for(idx=0, n=3; idx<NELEMENTS(initial_mappings); idx++, n++)
	{
		tlbentry ent = initial_mappings[idx];
		ent.mas0 |= MAS0_ENT(n);
		tlb_update(&ent);
	}
	/* disable remaining entries */
	disabled.mas0 = disabled.mas1 = disabled.mas2 = disabled.mas3 = 0;
	for(;n<16;n++)
	{
		disabled.mas0 |= MAS0_ENT(n);
		tlb_update(&disabled);
	}
}

/* for PCI address assignment */
static
uint32_t next_mmio = 0x80000000,
         next_io   = 0xe0000000;

/* prepare the PCI host bridge */
static
void setup_pci_host(void)
{
	/* Setup MMIO window 256 MB */
	out32x(CCSRBASE, 0x8c20, next_mmio>>12);
	out32x(CCSRBASE, 0x8c24, 0x00000000);
	out32x(CCSRBASE, 0x8c28, next_mmio>>12);
	out32x(CCSRBASE, 0x8c30, 0x8004401b);
	/* Setup IO port window 16 MB */
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

/* on entry only ROM, RAM, and CCSR are accessible */
void Init(void)
{
	uint16_t vend = pci_in16(0,0,0,0),
	         device = pci_in16(0,0,0,2);
	if(vend!=0x1957 || device!=0x30)
		return; /* wrong host bridge, something is really wrong here */

	setup_tlb(); /* PCI memory regions now accessible */

	/* TODO: enable I2C bus */
	setup_pci_host();
	walk_bus(0);
	prepare_tsi148();
	load_os(0x10000);
}
