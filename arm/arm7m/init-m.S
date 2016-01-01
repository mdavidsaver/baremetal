.thumb
.syntax unified

/* boot time vector table */
.section .text.start
.global _boot_reset
.align 2
_boot_reset:
.word _main_stack_top
.word _setup  /* Reset */
.word _stuck  /* NMI */
.word _stuck  /* HardFault */
.word _stuck  /* MemManage */
.word _stuck  /* BusFault */
.word _stuck  /* UsageFault */
.word _stuck  /* Reserved */
.word _stuck  /* SVCall */
.word _stuck  /* Debug Mon. */
.word _stuck  /* Reserved */
.word _stuck  /* PendSV */
.word _stuck  /* SysTick */
.word _stuck  /* Ext. IRQ 0 */
.word _stuck  /* Ext. IRQ 1 */
.word _stuck  /* Ext. IRQ 2 */

.section .text

/* vector table used after boot */
.global _vec_reset
.align 7 /* 128 bytes */
_vec_reset:
.word _main_stack_top
.word _setup  /* Reset */
.word _stuck_NM  /* NMI */
.word _stuck_HF  /* HardFault */
.word memfault_handler  /* MemManage */
.word _stuck_BF  /* BusFault */
.word _vec_usage /* UsageFault */
.word _stuck_RV  /* Reserved */
.word _stuck_RV  /* Reserved */
.word _stuck_RV  /* Reserved */
.word _stuck_RV  /* Reserved */
.word _stuck_SV  /* SVCall */
.word _stuck_DM  /* Debug Mon. */
.word _stuck_RV  /* Reserved */
.word _stuck_PS  /* PendSV */
.word _vec_systick  /* SysTick */
.word _stuck_EX  /* Ext. IRQ 0 */
.word _stuck_EX  /* Ext. IRQ 1 */
.word _stuck_EX  /* Ext. IRQ 2 */

.thumb_func
_stuck_NM : b _stuck_NM
.thumb_func
_stuck_HF : b _stuck_HF
.thumb_func
_stuck_BF : b _stuck_BF
.thumb_func
_stuck_UF : b _stuck_UF
.thumb_func
_stuck_RV : b _stuck_RV
.thumb_func
_stuck_SV : b _stuck_SV
.thumb_func
_stuck_DM : b _stuck_DM
.thumb_func
_stuck_PS : b _stuck_PS
.thumb_func
_stuck_ST : b _stuck_ST
.thumb_func
_stuck_EX : b _stuck_EX

.global _setup
.type _setup, function
.thumb_func
_setup:
.fnstart
.cantunwind
  cpsid if

  /* prepare C environment */
  bl _zero_bss
  bl _move_data

  ldr r0, =_proc_stack_top
  msr psp, r0

  mov r0, #(1<<9)|(1<<3) /* STKALIGN | UNALIGN_TRP */
  ldr r1,=0xe000ed14 /* CCR */
  str r0, [r1]

  /* set exception vector base address (needed when loaded to ram) */
  ldr r0, =_vec_reset
  ldr r1, =0xe000ed08 /* VTOR */
  str r0, [r1]

  /* Enable UsageFault, BusFault, and MemManage */
  mov r0, #(1<<18)|(1<<17)|(1<<16)
  ldr r1, =0xe000ed24 /* SHCSR */
  str r0, [r1]
  cpsie f

  adr r0, _early_string2
  blx puts

  blx __run_init /* run C++ static ctors */

  blx Init /* jump to "main" */

  blx __run_fini /* run C++ static dtors */

  /* fall through to halt */

.global abort
.type abort, function
.thumb_func
.align 4

.global halt
.type halt, function
.thumb_func
.align 4
abort:
halt:
  cpsid if
  ldr sp, =_main_stack_top

  blx systick_shutdown

  /* request shutdown */

  ldr r0, =_late_string1
  blx puts

  ldr r0, =0xe000ed0c /* AIRCR */
  ldr r1, =0x05fa0004 /* external reset */
  str r1, [r0]
  dsb

  /* fall through to spin */

.thumb_func
_stuck:
  b _stuck
.fnend
.size _setup, .-_setup
.size abort, .-abort
.size halt, .-halt


/* Assumes that .bss load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
_zero_bss:
  /* zero bss, aligned to 4 bytes */
  mov r0, #0
  ldr r1, =__bss_start
  ldr r2, =__bss_end

_bss_loop:
  cmp r1, r2
  itt lt
  strlt r0, [r1], #4  /* *r1++ = r0 where r1 is uint32_t* */
  blt _bss_loop

  bx lr

/* Assumes that .data load, start, and end addresses are aligned to 4 bytes.
 * This must be ensured in the linker script
 */
_move_data:
  /* move .data section to ram */
  ldr r0, =__data_load
  ldr r1, =__data_start

  /* if load==start then no move is necessary */
  cmp r0, r1
  it eq
  bxeq lr

  ldr r2, =__data_end

_data_loop:
  cmp r1, r2
  ittt lt
  ldrlt r3, [r0], #4
  strlt r3, [r1], #4
  blt _data_loop

  bx lr

.global _data_loop
.thumb_func
.align 2
_vec_usage:
  ldr r0, =_und_err_msg
  blx puts
  blx halt

.global _vec_systick
.thumb_func
.align 2
_vec_systick:
  push {lr}
  blx systick_handler
  pop {lr}
  bx lr

.global irq_mask
.thumb_func
irq_mask:
  mrs r0, faultmask
  cpsid i
  blx lr /* return 0 if IRQ was active, 1 if was masked */

.global irq_unmask
.thumb_func
irq_unmask:
  cmp r0, #0
  it ne
  blxne lr
  cpsie i
  blx lr

/* keep these strings in .text so they are always available */
_early_string1:
.string "booting...\n"
_early_string2:
.string "Enter Init()\n"
_late_string1:
.string "halt\n"

.section .rodata
_und_err_msg:
.string "UsageFault\n"

.section .bss
.align 4

.skip 0x400
_main_stack_top:

.skip 0x400
_proc_stack_top:
