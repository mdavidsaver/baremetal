.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

.macro make_vector name
.align 8
.global \name
\name:
  b \name
.endm

.section .text.start
.global _jump_start

_jump_start:
  b _start

make_vector _vec_chk
make_vector _vec_dstore
.align 7
.global _vec_dseg
_vec_dseg:
  b _vec_dseg
make_vector _vec_istore
.align 7
.global _vec_iseg
_vec_iseg:
  b _vec_iseg
make_vector _vec_ext
make_vector _vec_align
make_vector _vec_prog
make_vector _vec_nofpu
make_vector _vec_dec
.align 7
.global _vec_hdec
_vec_hdec:
  b _vec_hdec
make_vector _vec_res1
make_vector _vec_res2
make_vector _vec_sc
make_vector _vec_trace

.section .text.reset
.global _jump_start2
_jump_start2:
  b _jump_start

.section .text.os
.global _start
_start:
  b _start /* place holder for real entry point at 0x1000000 */
