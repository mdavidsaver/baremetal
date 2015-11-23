#define SPR_MAS0 624
#define SPR_MAS1 625
#define SPR_MAS2 626
#define SPR_MAS3 627

#define MAS0_TLB0 0
#define MAS0_TLB1 (1<<(63-35))
#define MAS0_ENT(N) (((N)&0xf)<<(63-47))

#define MAS1_V (1<<(63-32))
#define MAS1_TID(N) (((N)&0xff)<<(63-47))
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

.section .rodata
_tlb_data:
.long MAS0_TLB1|MAS0_ENT(0)
.long MAS1_V|MAS1_TSIZE(0x9)
.long MAS2_EPN(0)|MAS2_RAM
.long MAS3_RPN(0)|MAS3_RAM

.long MAS0_TLB1|MAS0_ENT(1)
.long MAS1_V|MAS1_TSIZE(0x7)
.long MAS2_EPN(0xff000000)|MAS2_ROM
.long MAS3_RPN(0xff000000)|MAS3_ROM

.long MAS0_TLB1|MAS0_ENT(2)
.long MAS1_V|MAS1_TSIZE(0x9)
.long MAS2_EPN(0x80000000)|MAS2_DEVICE
.long MAS3_RPN(0x80000000)|MAS3_DEVICE

.long MAS0_TLB1|MAS0_ENT(3)
.long MAS1_V|MAS1_TSIZE(0x9)
.long MAS2_EPN(0x81000000)|MAS2_DEVICE
.long MAS3_RPN(0x81000000)|MAS3_DEVICE

.long MAS0_TLB1|MAS0_ENT(4)
.long MAS1_V|MAS1_TSIZE(0x7)
.long MAS2_EPN(0xe0000000)|MAS2_DEVICE
.long MAS3_RPN(0xe0000000)|MAS3_DEVICE

.long MAS0_TLB1|MAS0_ENT(5)
.long MAS1_V|MAS1_TSIZE(0x5)
.long MAS2_EPN(0xe1000000)|MAS2_DEVICE
.long MAS3_RPN(0xff700000)|MAS3_DEVICE

.long MAS0_TLB1|MAS0_ENT(6)
.long MAS1_V|MAS1_TSIZE(0x7)
.long MAS2_EPN(0xe2000000)|MAS2_DEVICE
.long MAS3_RPN(0xe2000000)|MAS3_DEVICE

.long MAS0_TLB1|MAS0_ENT(7)
.long 0, 0, 0

.long MAS0_TLB1|MAS0_ENT(8)
.long 0, 0, 0
_tlb_data_end:

.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

.section .text
_setup_tlb:
  /* Early setup of TLB to allow access to RAM */

  load_const %r1, _tlb_data

  li %r2, (_tlb_data_end-_tlb_data)/16
  mtctr %r2

tlb_loop:

  lwz %r0, 0(%r1)
  mtspr SPR_MAS0, %r0
  lwz %r0, 4(%r1)
  mtspr SPR_MAS1, %r0
  lwz %r0, 8(%r1)
  mtspr SPR_MAS2, %r0
  lwz %r0, 12(%r1)
  mtspr SPR_MAS3, %r0

  sync
  isync
  tlbwe
  sync
  isync

  addi %r1,%r1,16
  bdnz tlb_loop

  b _start

.global load_os
load_os:
  load_const %r0, 0x10000
  mtlr %r0
  li %r0, 0
  li %r1, 0
  li %r2, 0
  li %r3, 0
  li %r4, 0
  li %r5, 0
  li %r6, 0
  li %r7, 0
  li %r8, 0
  li %r9, 0
  li %r10, 0
  li %r11, 0
  li %r12, 0
  li %r13, 0
  li %r14, 0
  li %r15, 0
  li %r16, 0
  li %r17, 0
  li %r18, 0
  li %r19, 0
  li %r20, 0
  li %r21, 0
  li %r22, 0
  li %r23, 0
  li %r24, 0
  li %r25, 0
  li %r26, 0
  li %r27, 0
  li %r28, 0
  li %r29, 0
  li %r30, 0
  li %r31, 0
  blr

/* The reset vector is the last instruction in the ROM,
 * so jump back immediately
 */
.section .text.init
.global _vec_reset
_vec_reset:
  b _setup_tlb
