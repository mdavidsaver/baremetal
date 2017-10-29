
PREFIX=/home/travis/.rtems/bin/powerpc-rtems4.10-
GCC=$(PREFIX)gcc
OBJCOPY=$(PREFIX)objcopy
SIZE=$(PREFIX)size

HOST_GCC=gcc

# use dwarf-2 as gdb 4.8 doesn't understand dwarf 4.
# Replace w/ '-g'
DEBUG=-gdwarf-2
#DEBUG=-g -O2

CFLAGS=-mcpu=powerpc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs $(DEBUG) -Wall -Wextra
LDFLAGS=-static

CFLAGS+=-Os

EXE += tomload investigate

TARGETS += test-ell

tomload_NAME = tomload.bin
tomload_LD = tomload.ld
tomload_OBJS += init.o init-tom.o tomload.o common.o uart.o fw_cfg.o pci.o ell.o

investigate_NAME = investigate.bin
investigate_OBJS += init.o init-reloc.o investigate.o common.o uart.o

# $(1) - EXE name
define exe_defs
$(1)_NAME ?= $(1)
$(1)_LD ?= os.ld
TARGETS += $($(1)_NAME)
CLEANS += $($(1)_NAME) $(1).elf $(1).map
OBJS += $($(1)_OBJS)
endef
$(foreach name,$(EXE),$(eval $(call exe_defs,$(name))))

DEPS = $(OBJS:%.o=%.d)

all: $(TARGETS)

-include $(DEPS)

# $(1) - EXE name
define exe_rules
$(1).elf : $($(1)_LD) $($(1)_OBJS)
endef
$(foreach name,$(EXE),$(eval $(call exe_rules,$(name))))

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CLEANS)

%.o: %.c Makefile
	$(GCC) -o $@ -c $< -MD $(CFLAGS) $($*_CFLAGS)

%.o: %.S Makefile
	$(GCC) -o $@ -c $< -MD $(CFLAGS) $($*_CFLAGS)

%.elf:
	$(GCC) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(CFLAGS) $(LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O binary $< $@

test-ell: test/test-ell.c ell.c ell.h Makefile
	$(HOST_GCC) -DHOST_BUILD -g -Wall -Wextra -I. -o $@ $(filter %.c,$^)
