#include <stdint.h>
#include <stddef.h>

#include "mmio.h"
#include "tlb.h"
#include "exc.h"
#include "pci.h"
#include "pci_def.h"
#include "fw_cfg.h"
#include "common.h"

#define NELEMENTS(ARR) (sizeof(ARR)/sizeof(ARR[0]))

static uint32_t image_addr = 0x10000;
void load_os(uint32_t, const uint32_t*); /* in bootos.S */

static uint8_t macaddr[6];

static
void setup_tlb(void)
{
    unsigned idx, way;
    for(idx=0; idx<256; idx++) {
        for(way=0; way<2; way++) {
            tlbentry ent = {0,0,0,0};
            ent.mas0 = MAS0_TLB0|MAS0_ENT(way);
            ent.mas1 = MAS2_EPN(idx<<12);
            tlb_update(&ent);
        }
    }
}

static
void setup_i2c(void)
{
    /* MOTLOAD leaves the controller enabled,
     * and RTEMS blindly assumes this is the case
     */
    out8x(CCSRBASE, 0x3008, 0x80);
}

static
void show_fw_cfg(void)
{
    uint32_t size;

    if(fw_cfg_show()) {
        printk("No FW Info found\n");
        return;
    }

    {
        uint64_t i64 = fw_cfg_read64(FW_CFG_RAM_SIZE);
        printk("Ram size %08x%08x\n", (unsigned)(i64>>32), (unsigned)i64);
    }

    {
        char cmdline[128];
        fw_cfg_read(FW_CFG_CMDLINE_DATA, cmdline, sizeof(cmdline));
        cmdline[sizeof(cmdline)-1] = '\0';
        printk("Append: \"%s\"\n", cmdline);
    }

    image_addr = fw_cfg_read32(FW_CFG_KERNEL_ENTRY);

    fw_cfg_list_files();

    if(!fw_cfg_open("tomload/vpd", &size)) {
        char buf[9];
        fw_cfg_readmore(buf, 8);
        buf[8] = '\0';
        size-=8;

        if(strcmp(buf, "MOTOROLA")!=0) {
            printk("Bad VPD\n");

        } else {
            uint32_t pos=0;
            char prev='\0';
            unsigned maccnt = 0;

            while(pos<size) {
                uint8_t id = fw_cfg_readbyte(),
                        len= fw_cfg_readbyte();
                pos += 2;
                if(id==0xff || id==0)
                    break;
                if(id==8 && len>=6 && maccnt++==0) {
                    // MAC Address
                    unsigned i;
                    for(i=0; i<6; i++) {
                        macaddr[i] = fw_cfg_readbyte();
                    }
                    for(; i<len; i++)
                        fw_cfg_readbyte();
                    printk("VPD Mac 0\n");

                } else {
                    printk("VPD %02x (%u) \"", id, len);
                    for(;len; len--) {
                        putc_escape(fw_cfg_readbyte());
                        pos++;
                    }
                    printk("\"\n");
                }
            }
            while(pos<0x10f0) {
                fw_cfg_readbyte();
                pos++;
            }
            printk("GEV:\n ");
            while(pos<size){
                char c = fw_cfg_readbyte();
                pos++;
                if(c=='\0' && prev=='\0')
                    break;
                else if(c=='\0')
                    printk("\n ");
                else
                    putc(c);
                prev = c;
            }
            putc('\n');
        }
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
void map_pci_interrupt(unsigned b, unsigned d, unsigned f, struct pci_info *info)
{
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
    (void)info;
    uint8_t pin = pci_in8(b,d,f, PCI_INTERRUPT_PIN),
            line = 0;
    if(b==0) {
        if(0) {}
        else if(d==0x11 && f==0 && pin) line = 0;
        else if(d==0x12 && f==0 && pin) line = 4;
    } else if(pin && (b==1 || b==2)) {
        line = (pin-1u+d)&3u;
        line += 4u;
    }

    printk("Assign IRQ %x:%x.%x pin=%u line=%u\n", b, d, f, pin, line);
    pci_out8(b, d, f, PCI_INTERRUPT_LINE, line);
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

static void setup_uart(unsigned n)
{
    uint32_t base = CCSRBASE + (n==0 ? 0x4500 : 0x4600);

    /* Setup for 9600 8N1
     *
     * Set baud rate divider (CCB=333MHz)
     *  333e6/(16*2170) = 9591 (~9600)
     */

    // LCR
    out8x(base, 3, 0x83);
    out8x(base, 1, 2170>>8);
    out8x(base, 0, 2170&0xff);

    out8x(base, 3, 0x03);
}

static
void set_mac(void)
{
    uint32_t addr1, addr2;

    addr2 = macaddr[1];
    addr2<<=8;
    addr2|= macaddr[0];
    addr2<<=16;

    addr1 = macaddr[5];
    addr1<<=8;
    addr1|= macaddr[4];
    addr1<<=8;
    addr1|= macaddr[3];
    addr1<<=8;
    addr1|= macaddr[2];

    if(!addr1 && !addr2)
        return;

    printk("Set MAC0 %08x %08x\n", (unsigned)addr1, (unsigned)addr2);

    /* place the default MAC where RTEMS expects it */
    out32x(CCSRBASE, 0x24540, addr1); // TSEC1 MACSTNADDR1
    out32x(CCSRBASE, 0x24544, addr2); // TSEC1 MACSTNADDR2
}

/* on entry only ROM, RAM, and CCSR are accessible */
void Init(void)
{
    setup_uart(0);
    setup_uart(1);

	uint16_t vend = pci_in16(0,0,0,0),
	         device = pci_in16(0,0,0,2);
	if(vend!=0x1957 || device!=0x30)
		return; /* wrong host bridge, something is really wrong here */

	setup_tlb(); /* PCI memory regions now accessible */
    printk("TOMLOAD\n");
    show_fw_cfg();

    setup_pci_host();

    {
        pci_info info = {
            .next_mmio = 0x80000000,
            .next_io   = 0xe0000000,
            .irqfn      = &map_pci_interrupt,
        };
        pci_setup(&info);
    }
    
	setup_i2c();

	prepare_tsi148();

    set_mac();

    printk("Load from %08x\n", (unsigned)image_addr);

    uint32_t regs[32];
    memset(regs, 0 ,sizeof(regs));
	load_os(image_addr, regs);
}

void os_return(void)
{
    printk("TOMLOAD: Image returns.  Halt.\n");
    out8x(0xe2000000, 1, in8x(0xe2000000, 1) | 0xa0);
    while(1) {}
}

void cpu_exception(unsigned num, const exc_frame* frame)
{
    unsigned show_dear = 0;
    uint32_t inst_addr;
    if(num==EXC_CRIT) {
        inst_addr = READ_SPR(SPR_CSRR0);
    } else if(num==EXC_MC) {
        inst_addr = READ_SPR(SPR_MCSRR0);
    } else {
        inst_addr = READ_SPR(SPR_SRR0);
    }

    uint32_t esr = READ_SPR(SPR_ESR);

    (void)frame;
    printk("TOMLOAD: exception %u @%08x ESR=%08x\n  ", num, (unsigned)inst_addr, (unsigned)esr);

    switch(num) {
    case EXC_CRIT: printk("Critial\n"); break;
    case EXC_MC:   printk("Machine Check\n"); break;
    case EXC_DS:   printk("Data Store\n"); show_dear = 1; break;
    case EXC_IS:   printk("Inst Store\n"); show_dear = 1; break;
    case EXC_EXT:  printk("External \n"); break;
    case EXC_ALIGN:printk("Alignment\n"); show_dear = 1; break;
    case EXC_PROG: printk("Program\n"); show_dear = 1; break;
    case EXC_NOFPU:printk("FPU\n"); break;
    case EXC_SYSCALL: printk("SYSCALL\n"); break;
    case EXC_NOAPU:printk("APU\n"); break;
    case EXC_DEC:  printk("Dec\n"); break;
    case EXC_TIMER:printk("Timer\n"); break;
    case EXC_WD:   printk("WD\n"); break;
    case EXC_DTLB: printk("Data TLB\n"); show_dear = 1; break;
    case EXC_ITLB: printk("Inst TLB\n"); show_dear = 1; break;
    case EXC_DEBUG:printk("Debug\n"); break;
    default:
        printk("Unknown\n");
    }

    if(show_dear) {
        printk("  Address %08x\n", (unsigned)READ_SPR(SPR_DEAR));
    }

    if(esr&ESR_PIL) {
        printk("  illegal instruction\n");
    }

    out8x(0xe2000000, 1, in8x(0xe2000000, 1) | 0xa0);
    while(1) {}
}
