TARGETS=lm3s6965evb host

all: $(TARGETS:%=all-%)
dep: $(TARGETS:%=dep-%)
clean:
	rm -rf $(TARGETS:%=build-%)

help:
	@echo "TARGETS: $(TARGETS)"
	@echo " make all|clean|help"
	@echo " make *-TARGET"

.PHONY: all dep clean help

define withtarg =
%-$(1): build-$(1)/Makefile
	$(MAKE) -C build-$(1) $$*
build-$(1)/Makefile:
	[ -d build-$(1) ] || install -d build-$(1)
	printf "BSP=$(1)\nTOP=..\ninclude ../Makefile.$(1)\n" > $$@
.PHONY: %-$(1)
endef

$(foreach targ,$(TARGETS),$(eval $(call withtarg,$(targ))))
