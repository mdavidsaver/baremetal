vpath %.c . ../common

PREFIX?=/usr/bin
GCC=$(firstword $(wildcard $(PREFIX)/*86*-*linux*-gcc))
OBJCOPY=$(firstword $(wildcard $(PREFIX)/*86*-*linux*-objcopy))

ifeq ($(GCC),)
$(error Failed to find gcc)
endif

CFLAGS=-m32 -march=i486 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -g -O2 -Wall -Wextra -Werror -I. -I../common

all: os.bin


os.elf: os.ld init.o main.o common.o

clean:
	rm -f init.o main.o os.elf os.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -c $< -o $@ $(CFLAGS)

%.elf:
	$(GCC) -T $*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(filter %.o,$^)

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
