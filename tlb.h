#ifndef TLB_H
#define TLB_H

#include "spr.h"

#define MAS0_TLB0 0
#define MAS0_TLB1 (1<<(63-35))
#define MAS0_ENT(N) (((N)&0xf)<<(63-47))

#define MAS1_V (1<<(63-32))
#define MAS1_IPROT (1<<(63-33))
#define MAS1_TID(N) (((N)&0xff)<<(63-47))
#define MAS1_TS(N) ((N)<<(63-51))
/* Size in bytes is 4**N
 * mpc8540 only support some sizes: N in [1,9] (4k -> 256MB)
 */
#define MAS1_TSIZE(N) (((N)&0xf)<<(63-55))

#define MAS2_EPN(ADR) ((ADR)&0xfffff000)
#define MAS2_X (3<<(63-58))
#define MAS2_W (1<<(63-59))
#define MAS2_I (1<<(63-60))
#define MAS2_M (1<<(63-61))
#define MAS2_G (1<<(63-62))
#define MAS2_E (1<<(63-63))
/* mem coherence */
#define MAS2_RAM  MAS2_M
/* mem coherence */
#define MAS2_ROM  MAS2_M
/* cache inhibit, mem coherence, guarded */
#define MAS2_DEVICE (MAS2_I|MAS2_M|MAS2_G)

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
