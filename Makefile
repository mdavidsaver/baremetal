TARGETS=lm3s6965evb

all: $(TARGETS:%=all-%)
clean:
	rm -rf $(TARGETS:%=build-%)

help:
	@echo "TARGETS: $(TARGETS)"
	@echo " make all|clean|help"
	@echo " make *-TARGET"

.PHONY: all clean help

define withtarg =
%-$(1):
	[ -d build-$(1) ] || install -d build-$(1)
	$(MAKE) -f ../Makefile.$(1) -C build-$(1) all TOP=.. BSP=$(1)
.PHONY: %-$(1)
endef

$(foreach targ,$(TARGETS),$(eval $(call withtarg,$(targ))))
