/* Minimal bare metal arm program for use without a bootloader.
 * Parts are specific to ARM versatileab emulation w/ QEMU
 *
 * Michael Davidsaver <mdavidsaver@gmail.com>
 */
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __SIZE_TYPE__ size_t;

extern char __bss_start, __bss_end;
extern char __data_start, __data_end, __data_load;

/* some variables to see if we're loading up the data and bss sections correctly */
volatile char *ioportbase;
int foobar;
volatile uint32_t ioportbase2 = 0xdeadbeef;
uint32_t foobar2 = 0xbad1face;

static inline
void memmove(void *dst, const void *src, size_t count)
{
  char *cdst = dst;
  const char *csrc = src;
  if(cdst<csrc) {
      /* move forward */
      while(count--) *cdst++=*csrc++;
  } else if(cdst>csrc) {
      /* move backwards */
      cdst += count;
      csrc += count;
      while(count--) *--cdst=*--csrc;
  }
}

static inline
void memset(void *dst, int val, size_t count)
{
  char *cdst = dst;
  while(count--) *cdst++=(char)val;
}

static inline
void uart_set(uint16_t offset, uint16_t value)
{
    volatile uint16_t *addr = (uint16_t*)(offset+0x101f1000);
    *addr = value;
}

static inline
uint16_t uart_get(uint16_t offset)
{
    volatile uint16_t *addr = (uint16_t*)(offset+0x101f1000);
    return *addr;
}

static inline
void putchar(char c)
{
    uart_set(0, c);
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

void Init(void)
{
    /* zero out BSS section */
    memset(&__bss_start, 0, &__bss_end-&__bss_start);
    /* copy from ROM to RAM.
     * regions may overlap
     */
    memmove(&__data_start, &__data_load, &__data_end-&__data_start);

    /* ARM isn't big on introspection, so no idea what peripherials to expect */

    /* defaults for PL011 UART work for QEMU */

    puts("hello world!\r\n");

    while(1) {}
}
