vpath %.c . ../common

PREFIX?=/usr/bin
GCC=$(firstword $(wildcard $(PREFIX)/arm-*eabi-gcc))
OBJCOPY=$(firstword $(wildcard $(PREFIX)/arm-*eabi-objcopy))

ifeq ($(GCC),)
$(error Failed to find gcc)
endif


CFLAGS=-ffreestanding -nostdlib -nostartfiles -nodefaultlibs -g -Wall -Wextra -Werror -I. -I../common
CFLAGS_A=-mcpu=cortex-a9
CFLAGS_M=-mcpu=cortex-m3 -mthumb

LDFLAGS=-static

all: os-a.bin os-m.bin

os-a.elf: vexpress.ld main-a.o init-a.o common-a.o
os-m.elf: cortexm.ld main-m.o init-m.o common-m.o

clean:
	rm -f *.o *.elf *.map *.bin *.img

%-m.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS_M) $(CFLAGS)

%-m.o: %-m.S
	$(GCC) -c $< -o $@ $(CFLAGS_M) $(CFLAGS)

%-a.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS_A) $(CFLAGS)

%-a.o: %-a.S
	$(GCC) -c $< -o $@ $(CFLAGS_A) $(CFLAGS)

%-m.elf:
	$(GCC) -T$(firstword $(filter %.ld,$^)) -Wl,-Map=$*.map -o $@ $(CFLAGS_M) $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%-a.elf:
	$(GCC) -T$(firstword $(filter %.ld,$^)) -Wl,-Map=$*.map -o $@ $(CFLAGS_A) $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@
