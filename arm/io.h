#ifndef IO_H
#define IO_H

#include <stdint.h>

#define mb() __asm__ ("dsb":::"memory")
#define rmb() mb()
#define wmb() mb()

static inline __attribute__((always_inline))
uint8_t in8(volatile void *addr)
{
    mb();
    return *(volatile uint8_t*)addr;
}

static inline __attribute__((always_inline))
uint16_t in16(volatile void *addr)
{
    mb();
    return *(volatile uint16_t*)addr;
}

static inline __attribute__((always_inline))
uint32_t in32(volatile void *addr)
{
    mb();
    return *(volatile uint32_t*)addr;
}

static inline __attribute__((always_inline))
void out8(volatile void *addr, uint8_t val)
{
    *(volatile uint8_t*)addr = val;
    mb();
}

static inline __attribute__((always_inline))
void out16(volatile void *addr, uint16_t val)
{
    *(volatile uint16_t*)addr = val;
    mb();
}

static inline __attribute__((always_inline))
void out32(volatile void *addr, uint32_t val)
{
    *(volatile uint32_t*)addr = val;
    mb();
}

#define rmw(N, ADDR, MASK, VAL) \
    do{uint##N##_t temp = in##N(ADDR);\
    temp&=~(MASK); temp|=(VAL)&(MASK);\
    out##N(ADDR, temp);\
    }while(0)

#define loop_until(N, ADDR, MASK, OP, VAL) \
    do{}while(!((in##N(ADDR)&(MASK))OP VAL))

#endif // IO_H
