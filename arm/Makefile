
PREFIX=arm-none-eabi-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

DEBUG=-g
OPT=-Os

# cortex a9 is armv7-a
ASFLAGS=-mcpu=cortex-a9 -W --fatal-warnings $(DEBUG)
CFLAGS=-mcpu=cortex-a9 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) $(OPT) -Wall -Wextra -Werror
LDFLAGS=-static

all: minimal-flash.img minimal-kern.bin timer-kern.bin os-kern.bin

OBJS = init.o common.o atag.o printk.o irq.o irq-setup.o mmu.o mmu-fault.o

minimal-kern.elf: vexpress.ld example-minimal.o init.o common.o atag.o printk.o
timer-kern.elf: vexpress.ld example-timer-irq.o init.o common.o atag.o printk.o irq.o irq-setup.o
os-kern.elf: vexpress.ld main.o $(OBJS)

minimal-flash.elf: vexpress-flash.ld boot-flash-minimal.o

boot-flash-%.o: %-kern.bin boot-flash.S
	$(GCC) -c boot-flash.S -o $@ $(CFLAGS) -DRAM_SIZE=64 -DBOARD_VEXPRESSA9 -DBIN_FILE="\"$<\""


clean:
	rm -f *.o *.elf *.map *.bin *.img

%.o: %.c Makefile
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -c $< -o $@ $(CFLAGS)

%.elf:
	$(GCC) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(CFLAGS) $($*_CFLAGS) $(LDFLAGS) $(*_LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

%.img: %.bin
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=64 conv=sparse
	dd if=$< of=$@ bs=1M conv=notrunc,sparse
