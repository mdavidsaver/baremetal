#!/bin/sh
set -x

die() {
  echo "$1" >&2
  exit 1
}

ORIGDIR="$PWD"

PREFIX=/usr/bin

if [ "$ARCH" != "i386"  ];then
  XDIR="/home/travis/.cache/x-tools/$ARCH"
  [ -e "$ORIGDIR/$ARCH.config" ] || die "Missing $ARCH.config"
  if [ ! -e "$XDIR/built" ]; then
    rm -rf "$XDIR"
    install -d "$XDIR"
    cd "$XDIR"

    git clone https://github.com/crosstool-ng/crosstool-ng.git
    cd crosstool-ng
    git reset --hard 6e7c61650a39a67ee02ed58c11d64c94c436bb33
    ./bootstrap
    ./configure --enable-local
    make -j2

    cp "$ORIGDIR/$ARCH.config" .config
    sed -i -e '/^CT_PREFIX_DIR=/d' .config
    echo "CT_PREFIX_DIR=$XDIR/usr" >> .config

    ./ct-ng build CT_PREFIX="$XDIR/usr" || (tail -n100 build.log; exit 1)
    touch "$XDIR/built"
  fi
  rm -rf "$XDIR/crosstool-ng"
  PREFIX="$XDIR/usr/bin"
  find "$PREFIX" -name '*-gcc'
fi

cd "$ORIGDIR"
echo "toolchain prefix $PREFIX"

make -C "$ARCH" PREFIX=$PREFIX
