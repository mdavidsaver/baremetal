#!/bin/sh
set -e -x

die() {
  echo "$1" >&2
  exit 1
}

QEMU="$QDIR/usr/bin/qemu-system-arm"
ARGS="-no-reboot -M lm3s6965evb -m 16 -serial stdio -display none -net nic -net user,restrict=on -d guest_errors,unimp"

$QEMU --version

RET=0

dotest() {
  echo "=============== Testing $1 ==============="
  TEST="-kernel $1"
  if perl tapit.pl "timeout 30 $QEMU $ARGS $TEST" </dev/null > test.log 2>&1
  then
    cat test.log
  else
    cat test.log
    echo "Test error, re-run with -d exec"
    timeout 30 $QEMU $ARGS $TEST </dev/null > test.log 2>&1 || true
    head -n50 test.log
    echo "==============="
    tail -n50 test.log
    RET=1
  fi
}

make PREFIX="$XDIR/usr/bin"

dotest test1-kern.bin
dotest test9-kern.bin
exit $RET
