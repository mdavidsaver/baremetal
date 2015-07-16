.arm

.section .text.start
.global _start
_start:
  ldr sp, =stack_top
  bl Init
_stuck:
  b _stuck

.section .bss
stack_bottom:
.skip 16384
stack_top:
