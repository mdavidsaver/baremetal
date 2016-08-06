# Multi-boot file header
# https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot_002dtime-configuration
.set MAGIC, 0x1BADB002
# image info, aligned, and pass mem_* fields
.set FLAGS, 0x10003
.set CHECKSUM, -(MAGIC+FLAGS)

.section .multiboot
.align 4
__mboot_head:
.long MAGIC
.long FLAGS
.long CHECKSUM
.long __mboot_head
.long __text_start
.long __data_end
.long __bss_end
.long _start

# Stack grows down
.section .bss.stack
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

.global get_eflags
get_eflags:
  pushf
  mov -4(%esi), %eax
  popf
  ret
