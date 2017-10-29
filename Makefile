
PREFIX=/home/travis/.rtems/bin/powerpc-rtems4.10-
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy

HOST_GCC=gcc

# use dwarf-2 as gdb 4.8 doesn't understand dwarf 4.
# Replace w/ '-g'
DEBUG=-gdwarf-2
#DEBUG=-g -O2

CFLAGS=-mcpu=powerpc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra
LDFLAGS=-static

CFLAGS+=-Os

all: tomload.bin investigate.bin test-ell

tomload.elf: Makefile init.S init-tom.S asm.h \
	tomload.c \
	common.c uart.c pci.c pci.h pci_def.h \
	fw_cfg.c fw_cfg.h \
	ell.c ell.h \
	common.h tlb.h mmio.h \
	tomload.ld

investigate.elf: Makefile init.S init-reloc.S asm.h \
	investigate.c \
	common.c common.h \
	uart.c \
	mmio.h \
	os.ld

clean:
	rm -f *.o
	rm -f *.elf *.bin *.map
	rm -f test-ell

%.elf:
	$(GCC) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.c,$^) $(filter %.S,$^) -lgcc

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

test-ell: test/test-ell.c ell.c ell.h
	$(HOST_GCC) -DHOST_BUILD -I. -o $@ $(filter %.c,$^) $(filter %.S,$^)
