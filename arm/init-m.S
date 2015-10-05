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

.global _setup
.type _setup, function
.thumb_func
_setup:

  /* prepare C environment */
  bl _zero_bss
  bl _move_data

  /* set exception vector base address (needed when loaded to ram) */
  ldr r0, =_vec_reset
  ldr r1, =0xe000ed08 /* VTOR */
  str r0, [r1]

  adr r0, _early_string2
  bl puts

  blx __run_init /* run C++ static ctors */

  blx Init /* jump to "main" */

  blx __run_fini /* run C++ static dtors */

  /* fall through to halt */

.global abort
.type abort, function
.align 4

.global halt
.type halt, function
.align 4
abort:
halt:
  ldr sp, =_main_stack_top

  /* request shutdown */

  ldr r0, =_late_string1
  bl puts

  ldr r0, =0xe000ed0c /* AIRCR */
  ldr r1, =0x05fa0004 /* external reset */
  str r1, [r0]
  dsb

  /* fall through to spin */

_stuck:
  b _stuck


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

/* keep these strings in .text so they are always available */
_early_string1:
.string "booting...\n"
_early_string2:
.string "Enter Init()\n"
_late_string1:
.string "halt\n"


.section .bss
.align 4

.skip 0x400
_main_stack_top:

.skip 0x400
_fiq_stack_top:
