TARGETS=vexpress-a9

all: $(TARGETS:%=all-%)
clean: $(TARGETS:%=clean-%)

all-%:
	[ -d build-$* ] || install -d build-$*
	$(MAKE) -f ../Makefile.$* -C build-$* all

clean-%:
	rm -rf build-$*

