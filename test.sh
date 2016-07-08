#!/bin/sh
set -x

die() {
  echo "$1" >&2
  exit 1
}

case "$ARCH" in
 i386)
   perl tapit.pl "timeout 10 qemu-system-i386 -no-reboot -m 128 -kernel i386/os.elf -display none -serial stdio"
   ;;
 powerpc)
   perl tapit.pl "timeout 10 qemu-system-ppc -no-reboot -m 128 -M prep -serial stdio -display none -bios powerpc/bios.bin -kernel powerpc/os.bin"
   ;;
 *) die "Unknown arch $ARCH";;
esac
