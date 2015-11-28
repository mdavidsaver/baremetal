.thumb
.syntax unified

.section .text.start
.global _boot_reset
.align 7
_boot_reset:
.word _main_stack_top
.word _start /* reset */
.word _vec_stuck
.word _vec_HF
.word _vec_stuck
.word _vec_stuck
.word _vec_UF
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_SVC
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck
.word _vec_stuck

.section .text

.global _start
.thumb_func
_start:
  /* zero bss */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end

_bss_loop:
  cmp r1, r2
  itt ne
  strbne r0, [r1], 1
  bne _bss_loop

  /* copy data */
  ldr r1, =__data_start
  ldr r2, =__data_end
  ldr r3, =__data_load

_data_loop:
  cmp r1, r2
  ittt ne
  ldrbne r0, [r3], 1
  strbne r0, [r1], 1
  bne _data_loop

  ldr r1, =0xe000e000
  ldr r0, =0x00070000
  str r0, [r1, #0xd24]

  ldr r0, =0x80000000
  str r0, [r1, #0xd1c]

  blx board_setup

  ldr r0, =in_main
  blx puts

  cpsie if

  svc 42

.global abort
.thumb_func
abort:
  ldr sp, =_main_stack_top

  ldr r0, =0x05fa0004 /* reset request */
  ldr r1, =0xe000e000
  str r0, [r1, #0xd0c] /* AIRCR */
  dsb

_stuck:
  b _stuck

.global putc
.thumb_func
putc:
  ldr r1, =0x4000c000

  cmp r0, #'\n'
  bne putc_loop
  push {r0-r2,lr}
  mov r0, #'\r'
  blx putc
  pop {r0-r2,lr}

putc_loop:
  ldr r2, [r1, 0x18]
  tst r2, #0x20
  bne putc_loop

  strb r0, [r1]
  bx lr

.global puts
.thumb_func
puts:
  push {lr}
  mov r3, r0
puts_loop:
  ldrb r0, [r3], #1
  cmp r0, #0
  it eq
  popeq {pc}
  blx putc
  b puts_loop

.thumb_func
_vec_stuck:
  mov r0, #'S'
  ldr r1, =0x4000c000
  strb r0, [r1]
  mrs r0, IPSR
  strb r0, [r1]
  bl abort

.thumb_func
_vec_HF:
  ldr r0, =bad_HF
  blx puts
  b abort

.thumb_func
_vec_UF:
  ldr r0, =in_UF
  blx puts
  b abort

/* omit .thumb_func */
_vec_SVC:
  ldr r0, =in_SVC
  blx puts
  bx lr

in_main:
.string "In Main\n"
bad_HF:
.string "Unexpected HF\n"
in_UF:
.string "In UsageFault\n"
in_SVC:
.string "In SVC, shouldn't be"

.section .bss
.align 4

.global _main_stack_bot
_main_stack_bot:
.skip 0x400
.global _main_stack_top
_main_stack_top:
