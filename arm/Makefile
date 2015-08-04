
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

all: minimal-flash.img os-kern.bin

OBJS = init.o common.o printk.o

minimal-flash.elf: os-flash.ld minimal.o $(OBJS)
os-kern.elf: os-kern.ld main.o $(OBJS) irq.o irq-setup.o

clean:
	rm -f *.o *.elf *.map *.bin *.img

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

%.elf:
	$(GCC) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

%.img: %.bin
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=64 conv=sparse
	dd if=$< of=$@ bs=1M conv=notrunc,sparse
