#include "asm.h"

#include "spr.h"
#include "tlb.h"

/* The mpc8540 ref. manual states that the initial tlb mapping
 * is only the high 4KB.
 * The mvme3100 programers reference states that the initial
 * mappings cover the whole ROM and CCSR.
 *
 * We setup TLB1 based on observed MOTLoad behavior.
 * Fill each of 16 entries with a 256MB mapping.
 * Covers entire address space.
 */
.section .text.early

_tlb_data_early:
.long MAS0_TLB1|MAS0_ENT(0)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xf0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xf0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(1)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xe0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xe0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(2)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xd0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xd0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(3)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xc0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xc0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(4)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xb0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xb0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(5)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0xa0000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0xa0000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(6)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x90000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x90000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(7)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x80000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x80000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(8)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x70000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x70000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(9)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x60000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x60000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(10)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x50000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x50000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(11)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x40000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x40000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(12)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x30000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x30000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(13)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x20000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x20000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(14)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x10000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x10000000)|MAS3_SR|MAS3_SW|MAS3_SX

.long MAS0_TLB1|MAS0_ENT(15)
.long MAS1_V|MAS1_IPROT|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0x00000000)|MAS2_I|MAS2_G
.long MAS3_RPN(0x00000000)|MAS3_SR|MAS3_SW|MAS3_SX

_tlb_data_early_end:

.section .text.early

/* At this point can access only .text.early
 */
_early_setup_tlb:
  load_const %r3, _tlb_data_early

  li %r4, (_tlb_data_early_end-_tlb_data_early)/16
  mtctr %r4 /* assume non-zero */

tlb_loop:
  bl tlb_update
  /* assume r3 and ctr unchanged */
  addi %r3, %r3, 16

  bdnz+ tlb_loop
  /* can now access ROM, RAM, and CCSR */

  /* update CCSRBAR with the sequence specified in mpc8540 ref. manual 4.3.1.1.1 */
  load_const %r3, 0xff700000 /* present location of CCSRBAR */
  load_const %r4, 0xe1000000>>12 /* the new location */
  load_const %r5, tlb_loop   /* an arbitrary non-CCSR address */

  lwz %r0, 0(%r3)
  isync
  stw %r4, 0(%r3)
  lwz %r0, 0(%r5)
  isync
  lwz %r0, 0(%r4)
  isync

  bl install_bad_exception

  b _pre_Init /* prepare eabi C environment then jump to Init() */


.global tlb_update
.type tlb_update, @function
tlb_update:
  lis %r0, 0

  lwz %r0, 0(%r3)
  mtspr SPR_MAS0, %r0
  lwz %r0, 4(%r3)
  mtspr SPR_MAS1, %r0
  lwz %r0, 8(%r3)
  mtspr SPR_MAS2, %r0
  lwz %r0, 12(%r3)
  mtspr SPR_MAS3, %r0

  sync
  isync
  tlbwe
  sync
  isync

  blr
.size tlb_update, .-tlb_update

/* The reset vector is the last instruction in the ROM,
 * so jump back immediately
 */
.section .text.init

.global _vec_reset
.global _start
_vec_reset:
_start:
  b _early_setup_tlb

/* place before other .text to make alignment easier */
.section .text.start

/* base of all exception vector */
.align 17
exception_base:

.macro def_vector N handler
/* each handler must be aligned to 16 bytes */
.align 4
vector\N:
   mtspr SPR_SPRG0, %r1 /* SPRG0 -> r1 */
   mflr %r1
   mtspr SPR_SPRG1, %r1 /* SPRG1 -> LR */
   li %r1, \N
   mtspr SPR_SPRG2, %r1 /* SPRG2 -> vector # */
   bl exc_prep
   bl \handler
.endm

def_vector 0 cpu_exception /* Critical */
def_vector 1 cpu_exception /* MachineCheck */
def_vector 2 cpu_exception /* DataStorage */
def_vector 3 cpu_exception /* InstructionStorage */
def_vector 4 cpu_exception /* ExternalInterrupt */
def_vector 5 cpu_exception /* Alignment */
def_vector 6 cpu_exception /* Program */
def_vector 7 cpu_exception /* NO FPU */
def_vector 8 cpu_exception /* syscall */
def_vector 9 cpu_exception /* NO APU */
def_vector 10 cpu_exception /* Decrementer */
def_vector 11 cpu_exception /* interval timer */
def_vector 12 cpu_exception /* watchdog timer */
def_vector 13 cpu_exception /* data TLB */
def_vector 14 cpu_exception /* instruction TLB */
def_vector 15 cpu_exception /* debug crit */

/* end of vectors */

exc_prep:
  load_const %r1, exc_reg_save /* see struct exc_frame in exc.h */

  /* GR[0-31] */
  stw %r0, 0(%r1)
  stmw %r2, 8(%r1)  /* store r2 -> r31 */
  mfspr %r0, SPR_SPRG0 /* r1 */
  stw %r0, 4(%r1)

  /* CR, CTR, LR */
  mfcr %r29
  mfctr %r30
  mfspr %r31, SPR_SPRG1 /* LR */
  stmw %r29, 8*32(%r1)

  mfspr %r3, SPR_SPRG2 /* vector # */
  mr %r4, %r1

  load_const %r1, exc_stack_top
  load_const %r2, _SDA2_BASE_
  load_const %r13, _SDA_BASE_

  blr

.macro set_vector N
    li %r3, vector\N - exception_base
    mtspr SPR_IVOR(\N), %r3
.endm

install_bad_exception:
  load_const %r3, exception_base
  mtspr SPR_IVPR, %r3

  set_vector 0
  set_vector 1
  set_vector 2
  set_vector 3
  set_vector 4
  set_vector 5
  set_vector 6
  set_vector 7
  set_vector 8
  set_vector 9
  set_vector 10
  set_vector 11
  set_vector 12
  set_vector 13
  set_vector 14
  set_vector 15

  blr

.section .bss

exc_reg_save:
.skip 4*32
exc_stack_bottom:
.skip 1024
.global stack_top
exc_stack_top:
