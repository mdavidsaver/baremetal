
PREFIX=powerpc-rtems4.10-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

# use dwarf-2 as gdb 4.8 doesn't understand dwarf 4.
# Replace w/ '-g'
DEBUG=-gdwarf-2
#DEBUG=-g -O2

# PREP is -mcpu=602
CFLAGS=-mcpu=powerpc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra
LDFLAGS=-static

all: os.bin bios.bin tomload.bin

os.elf: os.ld init.o main.o
bios.elf: bios.ld bios.o
tomload.elf: tomload.ld init.o tomload.o init-tom.o

clean:
	rm -f *.o
	rm -f *.elf *.bin *.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -o $@ -c $< $(CFLAGS)

%.elf:
	$(GCC) -T$*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
