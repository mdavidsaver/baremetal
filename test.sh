#!/bin/sh
set -x

die() {
  echo "$1" >&2
  exit 1
}

case "$ARCH" in
 i386)
   qemu-system-i386 --version
   perl tapit.pl "timeout 10 qemu-system-i386 -no-reboot -m 128 -kernel i386/os.elf -display none -serial stdio -d guest_errors,unimp"
   ;;
 powerpc)
   qemu-system-ppc --version
   if ! perl tapit.pl "timeout 30 qemu-system-ppc -no-reboot -m 128 -M prep -cpu 602 -serial stdio -display none -bios powerpc/bios.bin -kernel powerpc/os.bin -d guest_errors,unimp,exec" </dev/null > test.log 2>&1
   then
     echo "oops"
     tail -n100 test.log
     exit 1
   fi
   ;;
 *) die "Unknown arch $ARCH";;
esac
