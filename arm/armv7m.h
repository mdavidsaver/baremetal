#ifndef ARMV7m_h
#define ARMV7m_h

#include <stdint.h>

#define UART_DATA ((void*)0x4000c000)

typedef void (*vectfn)(void);

typedef struct {
	char *boot_stack;
	vectfn reset;
	vectfn nmi;
	vectfn hard;
	vectfn mem;
	vectfn bus;
	vectfn usage;
	uint32_t recv1[4];
	vectfn svc;
	vectfn debug;
	uint32_t recv2;
	vectfn pendsv;
	vectfn systick;
	vectfn irq[8];
} vect_table;

extern vect_table run_table;

static inline __attribute__((unused))
void out8(void *addr, uint8_t val)
{
	volatile uint8_t *A = addr;
	*A = val;
	__asm__ volatile ("dsb" ::: "memory");
}

static inline __attribute__((unused))
void out32(void *addr, uint32_t val)
{
	volatile uint32_t *A = addr;
	*A = val;
	__asm__ volatile ("dsb" ::: "memory");
}

static inline __attribute__((unused))
uint32_t in32(void *addr)
{
	volatile uint32_t *A = addr;
	__asm__ volatile ("dsb" ::: "memory");
	return *A;
}

void abort(void);

static inline __attribute__((unused))
void putc(char c)
{
	out8(UART_DATA, c);
}

static inline __attribute__((unused))
void puts(char *s)
{
	char c;
	while((c=*s++)!='\0')
		putc(c);
}

extern char hexchars[16];

static inline __attribute__((unused))
void puthex(uint32_t v)
{
	unsigned i;
	for(i=0; i<8; i++, v<<=4) {
		putc(hexchars[v>>28]);
	}
}

#endif /* ARMV7m_h */
