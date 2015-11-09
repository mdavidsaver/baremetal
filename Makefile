
PREFIX=arm-none-eabi-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

DEBUG=-g

ASFLAGS=-mcpu=cortex-m3 -W --fatal-warnings $(DEBUG)
CFLAGS=-mcpu=cortex-m3 -mthumb -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra -Werror
LDFLAGS=-static

all: test1-kern.bin
all: test2-kern.bin
all: test3-kern.bin
all: test4-kern.bin
all: test5-kern.bin
all: test6-kern.bin
all: test7-kern.bin

test1-kern.elf: cortexm.ld common.ld init-m.o test1.o
test2-kern.elf: cortexm.ld common.ld init-m.o test2.o
test3-kern.elf: cortexm.ld common.ld init-m.o test3.o
test4-kern.elf: cortexm.ld common.ld init-m.o test4.o
test5-kern.elf: cortexm.ld common.ld init-m.o test5.o
test6-kern.elf: cortexm.ld common.ld init-m.o test6.o
test7-kern.elf: cortexm.ld common.ld init-m.o test7.o

clean:
	rm -f *.o *.elf *.map *.bin *.img

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -c $< -o $@ $(CFLAGS)

%.elf:
	$(GCC) -T$(firstword $(filter %.ld,$^)) -Wl,-Map=$*.map -o $@ $(CFLAGS) $($*_CFLAGS) $(LDFLAGS) $(*_LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@

%.img: %.bin
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=64 conv=sparse
	dd if=$< of=$@ bs=1M conv=notrunc,sparse
