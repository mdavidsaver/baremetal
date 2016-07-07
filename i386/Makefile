PREFIX=/usr/bin
GCC=$(firstword $(wildcard $(PREFIX)/*86*-linux-gnu-gcc))

CFLAGS=-m32 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -g -O2 -Wall -Wextra -Werror

all: os.elf


os.elf: os.ld init.o main.o

clean:
	rm -f init.o main.o os.elf os.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(GCC) -c $< -o $@ $(CFLAGS)

%.elf:
	$(GCC) -T $*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(filter %.o,$^)
