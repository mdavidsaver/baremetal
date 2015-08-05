#ifndef MMU_H
#define MMU_H

#include "common.h"

#define MMU_READ  0x00
#define MMU_WRITE 0x01
//#define MMU_EXEC  0x02
#define MMU_PRIV  0x10
#define MMU_USER  0x00

void mmu_setup(void);

typedef struct {
    uint32_t vaddr, paddr;
    unsigned int lpae;
    unsigned int inner;
    unsigned int outer;
    unsigned int fault;
    unsigned int f:1;   /* fault */
    unsigned int nos:1; /* not outer sharable */
    unsigned int ns:1;  /* not secure */
    unsigned int sh:1;  /* sharable */
    unsigned int ss:1;  /* super section */
} mmu_test_result;

void mmu_test_access(uint32_t addr, unsigned mask, mmu_test_result*);

void mmu_test_print(unsigned t, const mmu_test_result*);

static inline __attribute__((unused))
void mmu_test(unsigned t, uint32_t addr, unsigned mask)
{
    mmu_test_result res;
    mmu_test_access(addr, mask, &res);
    mmu_test_print(t, &res);
}

#endif // MMU_H
