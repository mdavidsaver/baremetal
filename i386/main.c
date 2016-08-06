/* Minimal bare metal i386 program for use with multiboot enabled bootloader
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
#include <common.h>

extern char __text_start, __text_end,
            __rodata_start, __rodata_end,
            __data_start, __data_end,
            __bss_start, __bss_end;

typedef struct {
  uint32_t start, end;
  const char* name;
  uint32_t rsvd;
} boot_mod_t;

typedef struct {
    uint32_t size, base, skip1, length, skip2, mtype;
} mmap_info_t;

struct mb_info_t {
  uint32_t flags;
  uint32_t mem_lower, mem_upper;
  uint32_t boot_dev;
  const char* cmdline;
  uint32_t mods_count;
  const boot_mod_t* mods_addr;

  /* only ELF sym table option included, Ignoring a.out */

  uint32_t elf_num, elf_size, elf_addr, elf_shndx;

  uint32_t mmap_length;
  const char *mmap_addr;

  uint32_t drives_len, drives_addr;

  uint32_t config_table;

  /* ignore the reset
  uint32_t bootloadername;

  uint32_t apm_table;
  */
} mb_info;

struct bios_config_t {
    uint16_t size;
    uint8_t model, submodel, biosrev,
            feat1, feat2, feat3, feat4, feat5;
    
} bios_config;

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=r"(ret) : "Nd"(port) : "memory");
    return ret;
}

void outb(uint16_t port, uint8_t b)
{
  asm volatile ("outb %0, %1" :: "a"(b), "Nd"(port) : "memory");
}

void putchar(char c)
{
    outb(0x3f8, c);
}

static
void mb_report(void)
{
    if(mb_info.flags&(1<<0)) {
        printk("# Has mem\r\n#  %x -> %x\r\n", mb_info.mem_lower, mb_info.mem_upper);
    }

    if(mb_info.flags&(1<<1))
        printk("# Has boot dev %x\r\n", mb_info.boot_dev);

    if(mb_info.flags&(1<<2)) {
        printk("# Has cmdline: '%s'\r\n", mb_info.cmdline);
    }

    if(mb_info.flags&(1<<3)) {
        unsigned i;
        const boot_mod_t *mod = (void*)mb_info.mods_addr;
        printk("# Has boot modules 0x%x\r\n", mb_info.mods_count);
        for(i=0; i<mb_info.mods_count; i++) {
            printk("#  %x -> %x '%s'\r\n",
                   mod[i].start,
                   mod[i].end,
                   mod[i].name);
        }
    }

    if(mb_info.flags&(1<<4))
        puts("# Has a.out syms (not supported?!?)\r\n");

    if(mb_info.flags&(1<<5))
        puts("# Has ELF syms\r\n");

    if(mb_info.flags&(1<<6)) {
        const char *cur = mb_info.mmap_addr,
                   *end = cur+mb_info.mmap_length;
        printk("# Has MMAP %p %p\r\n", cur, end);
        while(cur<end) {
            const mmap_info_t *info = (const void*)cur;
            printk("#  %x -> %x %s (%x)\r\n",
                   info->base,
                   info->base+info->length-1,
                   info->mtype==1 ? "RAM " : "RSVD",
                   info->mtype);
            cur+=info->size+4;
        }
    }

    if(mb_info.flags&(1<<7))
        puts("# Has drive table\r\n");

    if(mb_info.flags&(1<<8)) {
        void *caddr = (void*)mb_info.config_table;
        printk("# Has config table %p\r\n", caddr);
    }

    if(mb_info.flags&(1<<9))
        puts("# Has loader name\r\n");

    if(mb_info.flags&(1<<10))
        puts("# Has APM table\r\n");

    if(mb_info.flags&(1<<11))
        puts("# Has VBE info\r\n");
}

/* some variables to see if the data and bss sections are correctly initialized */
int foobar;
uint32_t foobar2 = 0xbad1face;

void Init(uint32_t mb_magic, const void* pmb)
{
  /* assume UART was left configured by the bootloader */
  testInit(3);
  printk("# TEXT %p -> %p\r\n", &__text_start, &__text_end);
  printk("# RO   %p -> %p\r\n", &__rodata_start, &__rodata_end);
  printk("# DATA %p -> %p\r\n", &__data_start, &__data_end);
  printk("# BSS  %p -> %p\r\n", &__bss_start, &__bss_end);
  testeq32(mb_magic, 0x2badb002);
  printk("# data %p\r\n", pmb);
  if(mb_magic==0x2badb002) {
    puts("# Found multiboot\r\n");
    memcpy(&mb_info, pmb, sizeof(mb_info));
    printk("# flags %x\r\n", mb_info.flags);
    mb_report();
  }
  testeq32(foobar, 0);
  testeq32(foobar2, 0xbad1face);
  puts("# Goodbye\r\n");
  outb(0x64, 0xfe); /* ask the KBC to reset us */
  while(1) {}
}
