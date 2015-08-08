
PREFIX=arm-none-eabi-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

DEBUG=-g

# cortex a9 is armv7-a
ASFLAGS=-mcpu=cortex-a9 -W --fatal-warnings $(DEBUG)
CFLAGS=-mcpu=cortex-a9 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra -Werror
LDFLAGS=-static

all: os-flash.img os-kern.bin

os-flash.elf: vexpress-flash.ld init.o main.o
os-kern.elf: vexpress-kern.ld init.o main.o

clean:
	rm -f *.o *.elf *.map *.bin *.img

%.o: %.c
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
