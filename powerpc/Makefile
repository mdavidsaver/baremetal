
PREFIX=powerpc-linux-gnu-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

# use dwarf-2 as gdb 4.8 doesn't understand dwarf 4.
# Replace w/ '-g'
DEBUG=-gdwarf-2
#DEBUG=-g -O2

ASFLAGS=$(DEBUG)
# PREP is -mcpu=602
CFLAGS=-mcpu=powerpc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra
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
