
CROSS_COMPILE=powerpc-linux-gnu-
GCC=$(CROSS_COMPILE)gcc
OBJCOPY=$(CROSS_COMPILE)objcopy
SIZE=$(CROSS_COMPILE)size

HOST_GCC=gcc

CFLAGS+=-mcpu=8540 -msoft-float

# Don't use OS headers/libraries
STANDALONE=-ffreestanding -nostdlib -nostartfiles -nodefaultlibs
CFLAGS += $(STANDALONE)
LDFLAGS += $(STANDALONE)

CFLAGS+=-g

CFLAGS+=-Wall -Wextra -Werror

CFLAGS+=-Os

LDFLAGS+=-static

# newer ld will allow us to complain instead of just
# jumping orphaned sections somewhere random...
LDFLAGS+=-Wl,--orphan-handling=error

CFLAGS+=$(USR_CFLAGS)
LDFLAGS+=$(USR_LDFLAGS)

EXE += tomload investigate

TARGETS += test-ell
TARGETS += test-printk

tomload_NAME = tomload.bin
tomload_LD = tomload.ld
tomload_OBJS += init.o init-tom.o
tomload_OBJS += tomload.o bootos.o
tomload_OBJS += eabi.o common.o uart.o
tomload_OBJS += printk.o
tomload_OBJS += fw_cfg.o pci.o ell.o

investigate_NAME = investigate.bin
investigate_OBJS += init.o init-reloc.o
investigate_OBJS += investigate.o
investigate_OBJS += eabi.o common.o uart.o
investigate_OBJS += printk.o

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
	rm -f $(TARGETS)
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CLEANS)

%.o: %.c Makefile
	$(GCC) -o $@ -c $< -MD $(CFLAGS) $($*_CFLAGS)

%.o: %.S Makefile
	$(GCC) -o $@ -c $< -MD $(CFLAGS) $($*_CFLAGS)

%.elf:
	$(GCC) -T$(filter %.ld,$^) -Wl,-Map=$*.map -o $@ $(LDFLAGS) $($*_LDFLAGS) $(filter %.o,$^) -lgcc

%.bin: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O binary $< $@

test-ell: test/test-ell.c ell.c ell.h Makefile
test-printk: test/test-printk.c printk.c

test-ell test-printk:
	$(HOST_GCC) -DHOST_BUILD -g -Wall -Wextra -I. -o $@ $(filter %.c,$^)
