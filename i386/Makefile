
PREFIX=i586-linux-gnu-
AS=$(PREFIX)as
GCC=$(PREFIX)gcc

ASFLAGS=-g
CFLAGS=-m32 -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -g -O2 -Wall -Wextra

all: os.elf


os.elf: os.ld init.o main.o

clean:
	rm -f init.o main.o os.elf os.map

%.o: %.c
	$(GCC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) $< -o $@ $(ASFLAGS)

%.elf:
	$(GCC) -T $*.ld -Wl,-Map=$*.map -o $@ $(CFLAGS) $(filter %.o,$^) -lgcc
