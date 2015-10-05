
PREFIX=arm-none-eabi-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
GXX=$(PREFIX)g++
AR=$(PREFIX)ar
OBJCOPY=$(PREFIX)objcopy

DEBUG=-g
OPT=-Os

# cortex a9 is armv7-a
ASFLAGS=-mcpu=cortex-a9 -W --fatal-warnings $(DEBUG)
CFLAGS=-mcpu=cortex-a9 -ffreestanding -nodefaultlibs $(DEBUG) $(OPT) -Wall -Wextra -Werror
LDFLAGS=-static

all: minimal-flash.img minimal-kern.bin timer-kern.bin mmu-kern.bin thread-kern.bin
all: cxx-kern.bin os-kern.bin

OBJS = init.o common.o atag.o printk.o irq.o irq-gic.o
OBJS += mmu.o mmu-fault.o page-alloc.o thread.o thread-switch.o systick.o

minimal-kern.elf: vexpress.ld example-minimal.o libos.a
timer-kern.elf: vexpress.ld example-timer-irq.o libos.a
mmu-kern.elf: vexpress.ld example-mmu.o libos.a
thread-kern.elf: vexpress.ld example-thread.o libos.a
cxx-kern.elf: vexpress.ld example-cxx.o example-cxx2.o libos.a
os-kern.elf: vexpress.ld main.o libos.a

minimal-flash.elf: vexpress-flash.ld boot-flash-minimal.o

libos.a: $(OBJS) magic-crt0

magic-crt0: crt0.o

.PHONY: magic-crt0

boot-flash-%.o: %-kern.bin boot-flash.S
	$(GCC) -c boot-flash.S -o $@ $(CFLAGS) -DRAM_SIZE=64 -DBOARD_VEXPRESSA9 -DBIN_FILE="\"$<\""


clean:
	rm -f *.o *.elf *.map *.bin *.img *.a

%.o: %.c Makefile
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.cpp Makefile
	$(GXX) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -c $< -o $@ $(CFLAGS)

%.a:
	rm -f $@.tmp
	$(AR) rcsD $@.tmp $(filter %.o,$^)
	mv $@.tmp $@

%.elf:
	$(GXX) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(CFLAGS) $($*_CFLAGS) $(LDFLAGS) $(*_LDFLAGS) $(filter %.o,$^) $(filter %.a,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

%.img: %.bin
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=64 conv=sparse
	dd if=$< of=$@ bs=1M conv=notrunc,sparse

.PRECIOUS: %.bin %.img
