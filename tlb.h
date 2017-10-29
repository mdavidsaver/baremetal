#ifndef TLB_H
#define TLB_H

#include "spr.h"

#define MAS0_TLB0 0
#define MAS0_TLB1 (1<<(63-35))
#define MAS0_ENT(N) (((N)&0xf)<<(63-47))

#define MAS1_V (1<<(63-32))
/* TODO, what does TS set mean? */
#define MAS1_DS 0
#define MAS1_IS 0
#define MAS1_TID(N) (((N)&0xff)<<(63-47))
/* Size in bytes is 4**N
 * e500 only support some sizes: N in [1,9] (4k -> 256MB)
 */
#define MAS1_TSIZE(N) (((N)&0xf)<<(63-55))

#define MAS2_EPN(ADR) ((ADR)&0xfffff000)
/* mem coherence */
#define MAS2_RAM  0x4
/* mem coherence */
#define MAS2_ROM  0x4
/* cache inhibit, mem coherence, guarded */
#define MAS2_DEVICE 0xe

#define MAS3_RPN(ADR) ((ADR)&0xfffff000)
/* All RWX */
#define MAS3_RAM 0x3f
/* All RX, no write */
#define MAS3_ROM 0x33
/* ALL RW, no exec */
#define MAS3_DEVICE 0x0f

#ifndef __ASSEMBLER__
#include <stdint.h>

typedef struct {
	uint32_t mas0;
	uint32_t mas1;
	uint32_t mas2;
	uint32_t mas3;
} tlbentry;

void tlb_update(const tlbentry *ent);
#endif

#endif /* TLB_H */
