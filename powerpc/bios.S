
.macro load_const rD, expr
  lis \rD, \expr@h
  ori \rD, \rD, \expr@l
.endm

.section .text.start
.global _jump_start

_jump_start:
  load_const %r3, 0x1000000
  mtlr %r3
  li %r0, 0
  blr
