TARGETS=vexpress-a9 lm3s6965evb

all: $(TARGETS:%=all-%)
clean: $(TARGETS:%=clean-%)

all-%:
	[ -d build-$* ] || install -d build-$*
	$(MAKE) -f ../Makefile.$* -C build-$* all BSP=$*

clean-%:
	rm -rf build-$*

