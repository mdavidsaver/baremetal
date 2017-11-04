/* cleanup registers and exit bootloader */
.global load_os
.type load_os, @function
load_os:
  mtlr %r3 /* OS entry point address */
  sub %r31, %r4, 4
  lwzu %r0, 4(%r31)
  lwzu %r1, 4(%r31)
  lwzu %r2, 4(%r31)
  lwzu %r3, 4(%r31)
  lwzu %r4, 4(%r31)
  lwzu %r5, 4(%r31)
  lwzu %r6, 4(%r31)
  lwzu %r7, 4(%r31)
  lwzu %r8, 4(%r31)
  lwzu %r9, 4(%r31)
  lwzu %r10, 4(%r31)
  lwzu %r11, 4(%r31)
  lwzu %r12, 4(%r31)
  lwzu %r13, 4(%r31)
  lwzu %r14, 4(%r31)
  lwzu %r15, 4(%r31)
  lwzu %r16, 4(%r31)
  lwzu %r17, 4(%r31)
  lwzu %r18, 4(%r31)
  lwzu %r19, 4(%r31)
  lwzu %r20, 4(%r31)
  lwzu %r21, 4(%r31)
  lwzu %r22, 4(%r31)
  lwzu %r23, 4(%r31)
  lwzu %r24, 4(%r31)
  lwzu %r25, 4(%r31)
  lwzu %r26, 4(%r31)
  lwzu %r27, 4(%r31)
  lwzu %r28, 4(%r31)
  lwzu %r29, 4(%r31)
  lwzu %r30, 4(%r31)
  lwz  %r31, 4(%r31)
  blrl
  /* reached if OS image returns to us */
  bl _setup_eabi
  bl os_return
spin:
  b spin
.size load_os, .-load_os
