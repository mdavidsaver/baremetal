# Multi-boot file header
# https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot_002dtime-configuration
.set MAGIC, 0x1BADB002
.set FLAGS, 3 # aligned and pass mem_* fields
.set CHECKSUM, -(MAGIC+FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Stack grows down
.section .bootstack
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
_start:
  # initialize stack pointer
  movl $stack_top, %esp
  pushl $0
  popf /* reset eflags */

  /* function arguments */
  pushl %ebx /* pmb */
  pushl %eax /* mb_magic */

  call Init
  cli
hang:
  hlt
  jmp hang
