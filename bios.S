.section .text.start
.global _jump_start

_jump_start:
  b _start

.section .text.os
.global _start
_start:
  b _start /* place holder for real entry point at 0x1000000 */
