
PREFIX=powerpc-rtems4.9-
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

all: os.bin bios.bin

os.elf: os.ld init.o main.o
bios.elf: bios.ld bios.o

clean:
	rm -f *.o
	rm -f os.elf os.bin os.map
	rm -f bios.elf bios.bin bios.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

%.elf:
	$(GCC) -T$*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
