#!/bin/sh
set -x

die() {
  echo "$1" >&2
  exit 1
}

qarch="$ARCH"
args=""
case "$ARCH" in
 i386)
   args="-m 128 -kernel i386/os.elf"
   ;;
 powerpc)
   qarch="ppc"
   args="-m 128 -M prep -cpu 602 -bios powerpc/bios.bin -kernel powerpc/os.bin"
   ;;
 arm)
   args="-m 128 -M vexpress-a9 -kernel arm/os-a.bin"
   ;;
 *) die "Unknown arch $ARCH";;
esac

qemu-system-${qarch} --version

if perl tapit.pl "timeout 30 qemu-system-$qarch -no-reboot $args -serial stdio -display none -net none -d guest_errors,unimp" </dev/null > test.log 2>&1
then
  cat test.log
else
  cat test.log
  echo "Test error, re-run with -d exec"
  timeout 30 qemu-system-$qarch -no-reboot $args -serial stdio -display none -net none -d guest_errors,unimp,exec </dev/null > test.log 2>&1 || true
  head -n100 test.log
  echo "==============="
  tail -n100 test.log
  exit 1
fi
