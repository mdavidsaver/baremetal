#ifndef SPR_H
#define SPR_H

#define SPR_IVPR 63
#define SPR_PVR 287

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
