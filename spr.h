#ifndef SPR_H
#define SPR_H

#define SPR_DEC 22
#define SPR_DECAR 54
#define SPR_TBL 284
#define SPR_TBH 285
#define SPR_TCR 340
#define SPR_TSR 336

#define SPR_SRR0 26
#define SPR_SRR1 27

#define SPR_CSRR0 58
#define SPR_CSRR1 59

#define SPR_ESR 62
#define ESR_PIL (1u<<(63-36))
#define ESR_PPR (1u<<(63-37))
#define ESR_PTR (1u<<(63-38))
#define ESR_ST  (1u<<(63-40))
#define ESR_DLK (1u<<(63-42))
#define ESR_ILK (1u<<(63-43))
#define ESR_BO  (1u<<(63-46))
#define ESR_SPE (1u<<(63-53))

#define SPR_DEAR 61

#define SPR_MCAR 573
#define SPR_MCSR 572
#define SPR_MCSRR0 570
#define SPR_MCSRR1 571

#define SPR_IVPR 63

#define SPR_PVR 287
/* SCR aka. SVR */
#define SPR_SCR 1023

#define SPR_IVOR(N) ((400)+(N))

#define SPR_SVR 1023

#define SPR_HID0 1008
#define SPR_HID1 1009
#define SPR_MMUCFG 1015

#define SPR_TLB0CFG 688
#define SPR_TLB1CFG 689

#define SPR_MAS0 624
#define SPR_MAS1 625
#define SPR_MAS2 626
#define SPR_MAS3 627
#define SPR_MAS4 628
#define SPR_MAS6 630

#define SPR_PID0 48
#define SPR_PID1 633
#define SPR_PID2 634

#define SPR_L1CSR0 1010
#define SPR_L1CSR1 1011

/* SPRG0-2 supervisor only
 * 3-7 user read
 */
#define SPR_SPRG0 272
#define SPR_SPRG1 273
#define SPR_SPRG2 274
#define SPR_SPRG3 275
#define SPR_SPRG4 276
#define SPR_SPRG5 277
#define SPR_SPRG6 278
#define SPR_SPRG7 279

#define SPR_USPRG0 256

#ifndef __ASSEMBLER__

#define READ_SPRx(NUM) ({uint32_t val; \
    __asm__ volatile ("mfspr %0, " #NUM : "=r"(val)::"memory"); \
    val;})

#define WRITE_SPRx(NUM, VAL) do{uint32_t val = (VAL); \
    __asm__ volatile ("mtspr " #NUM ", %0" ::"r"(val):"memory"); \
    }while(0)

#define WRITE_SPR(A,B) WRITE_SPRx(A,B)
#define READ_SPR(A) READ_SPRx(A)

#endif /* __ASSEMBLER__ */

#endif /* SPR_H */
