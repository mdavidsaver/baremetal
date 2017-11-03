#include "asm.h"

#include "spr.h"
#include "tlb.h"

/* The mpc8540 ref. manual states that the initial tlb mapping
 * is only the high 4KB.
 * The mvme3100 programers reference states that the initial
 * mappings cover the whole ROM and CCSR.
 *
 * We take a conservative approach and assume only the high 4KB,
 * which contains only .text.early and .text.init, are accessible.
 * Impose the _tlb_data_early mappings before entering C code.
 */
.section .text.early


/* TLB0 provies 2 entires, TLB1 provides 16 entries.
 * Each with a max. size of 256MB (16*256MB=4GB)
 */

_tlb_data_early:
/* early boot TLB configuration.
 * access to RAM, ROM and CCSR w/ TLB1
 * clear TLB0.
 *
 * Map the ROM first as this is where we execute from.
 */
.long MAS0_TLB1|MAS0_ENT(0)
.long MAS1_V|MAS1_TSIZE(0x6) /* 4 MB */
.long MAS2_EPN(0xffc00000)|MAS2_ROM
.long MAS3_RPN(0xffc00000)|MAS3_ROM

.long MAS0_TLB1|MAS0_ENT(1)
.long MAS1_V|MAS1_TSIZE(0x6) /* 4 MB */
.long MAS2_EPN(0xff800000)|MAS2_ROM
.long MAS3_RPN(0xff800000)|MAS3_ROM

/* RAM */
.long MAS0_TLB1|MAS0_ENT(2)
.long MAS1_V|MAS1_TSIZE(0x9) /* 256 MB */
.long MAS2_EPN(0)|MAS2_RAM
.long MAS3_RPN(0)|MAS3_RAM

/* CCSR before relocation */
.long MAS0_TLB1|MAS0_ENT(3)
.long MAS1_V|MAS1_TSIZE(0x5) /* 1 MB */
.long MAS2_EPN(0xff700000)|MAS2_DEVICE
.long MAS3_RPN(0xff700000)|MAS3_DEVICE

/* CCSR after relocation */
.long MAS0_TLB1|MAS0_ENT(4)
.long MAS1_V|MAS1_TSIZE(0x5) /* 1 MB */
.long MAS2_EPN(0xe1000000)|MAS2_DEVICE
.long MAS3_RPN(0xe1000000)|MAS3_DEVICE

/* Disable TLB0 */

.long MAS0_TLB0|MAS0_ENT(0)
.long 0, 0, 0

.long MAS0_TLB0|MAS0_ENT(1)
.long 0, 0, 0

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

.section .text

.type bad_exception, @function
bad_exception:
  bl _setup_eabi
  bl os_exception
  b spin
.size bad_exception, .-bad_exception

install_bad_exception:
  load_const %r3, bad_exception
  mtspr SPR_IVPR, %r3
  lis %r3, 0
  mtspr SPR_IVOR(0), %r3
  mtspr SPR_IVOR(1), %r3
  mtspr SPR_IVOR(2), %r3
  mtspr SPR_IVOR(3), %r3
  mtspr SPR_IVOR(4), %r3
  mtspr SPR_IVOR(5), %r3
  mtspr SPR_IVOR(6), %r3
  mtspr SPR_IVOR(7), %r3
  mtspr SPR_IVOR(8), %r3
  mtspr SPR_IVOR(9), %r3
  mtspr SPR_IVOR(10), %r3
  mtspr SPR_IVOR(11), %r3
  mtspr SPR_IVOR(12), %r3
  mtspr SPR_IVOR(13), %r3
  mtspr SPR_IVOR(14), %r3
  mtspr SPR_IVOR(15), %r3

  blr

/* cleanup registers and exit bootloader */
.global load_os
.type load_os, @function
load_os:
  mtlr %r3 /* OS entry point address */
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
  blrl
  /* reached if OS image returns to us */
  bl _setup_eabi
  bl os_return
spin:
  b spin
.size load_os, .-load_os
