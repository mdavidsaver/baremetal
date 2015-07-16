
PREFIX=arm-none-eabi-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

DEBUG=-g

ASFLAGS=$(DEBUG)
CFLAGS=-mcpu=arm926ej-s -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra
LDFLAGS=-static

all: os.bin

os.elf: os.ld init.o main.o

clean:
	rm -f init.o main.o os.elf os.bin os.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

%.elf:
	$(GCC) -T$*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -S -O binary $< $@
