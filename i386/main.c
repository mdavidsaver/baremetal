/* Minimal bare metal i386 program for use with multiboot enabled bootloader
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __SIZE_TYPE__ size_t;

struct mb_info_t {
  uint32_t flags;
  uint32_t mem_lower, mem_upper;
  uint32_t boot_dev;
  const char* cmdline;
  uint32_t mods_count, mods_addr;

  /* only ELF sym table option included, Ignoring a.out */

  uint32_t elf_num, elf_size, elf_addr, elf_shndx;

  uint32_t mmap_length, mmap_addr;

  /* ignore the reset
  uint32_t drives_len, drives_addr;

  uint32_t rom_table;

  uint32_t bootloadername;

  uint32_t apm_table;
  */
} mb_info;

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=r"(ret) : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t b)
{
  asm volatile ("outb %0, %1" :: "a"(b), "Nd"(port));
}

static __attribute__((unused))
void memcpy(void *dst, const void *src, size_t count)
{
  char *cdst = dst;
  const char *csrc = src;
  while(count--) *cdst++=*csrc++;
}

static inline
void putchar(char c)
{
    outb(0x3f8, c);
}

static
void puts(const char* msg)
{
    char c;
    while( (c=*msg++) )
    {
        putchar(c);
    }
}

static __attribute__((unused))
void putval(uint32_t v)
{
    static char hex[] = "0123456789ABCDEF";
    uint8_t n = sizeof(v)*2;

    while(n--) {
        putchar(hex[v>>28]);
        v<<=4;
    }
}

/* some variables to see if the data and bss sections are correctly initialized */
int foobar;
uint32_t foobar2 = 0xbad1face;

void Init(uint32_t mb_magic, const void* pmb)
{
  /* assume UART was left configured by the bootloader */
  if(mb_magic==0x2badb002) {
    puts("Found multiboot\r\n");
    memcpy(&mb_info, pmb, sizeof(mb_info));
    if(mb_info.flags&0x1)
      puts(" Has mem\r\n");
    if(mb_info.flags&0x2)
      puts(" Has boot dev\r\n");
    if(mb_info.flags&0x4) {
      puts(" Has cmdline: '");
      puts(mb_info.cmdline);
      puts("'\r\n");
    }
    if(mb_info.flags&0x8)
      puts(" Has a.out syms (not supported?!?)\r\n");
    if(mb_info.flags&0x10)
      puts(" Has ELF syms\r\n");
    if(mb_info.flags&0x20)
      puts(" Has MMAP\r\n");
    if(mb_info.flags&0x40)
      puts(" Has drive table\r\n");
    if(mb_info.flags&0x80)
      puts(" Has config table\r\n");
    if(mb_info.flags&0x10)
      puts(" Has loader name\r\n");
    if(mb_info.flags&0x20)
      puts(" Has APM table\r\n");
    if(mb_info.flags&0x40)
      puts(" Has VBE info\r\n");
  }
  puts("hello world\r\nfoobar=");
  putval(foobar);
  puts("\r\nfoobar2=");
  putval(foobar2);
  puts("\r\n");
  while(1) {}
}
