
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

CFLAGS+=-Os

all: os.bin bios.bin tomload.bin powerdna.img

os.elf: init.S main.c
bios.elf: bios.S
tomload.elf: init.S tomload.c init-tom.S tlb.h mmio.h
powerdna.elf: init.S powerdna.c

clean:
	rm -f *.o
	rm -f *.elf *.bin *.map

%.elf: %.ld
	$(GCC) -T$*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.c,$^) $(filter %.S,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.img: %.bin
	gzip -9c $< > $*.gz
	mkimage -A ppc -O rtems -T kernel -a 0x1000000 -e 0x1000000 -n RTEMS -d $*.gz $@
