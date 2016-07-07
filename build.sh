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

    #curl http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.22.0.tar.xz | tar -xJ

    git clone https://github.com/crosstool-ng/crosstool-ng.git
    cd crosstool-ng
    git reset --hard 6e7c61650a39a67ee02ed58c11d64c94c436bb33
    ./bootstrap
    ./configure --prefix "$XDIR/usr"
    make -j2
    make install

    CTDIR="$PWD"

    mkdir ../ct
    cd ../ct

    cp "$ORIGDIR/$ARCH.config" .config
    PATH=$CTDIR:$PATH ct-ng build CT_PREFIX="$XDIR/usr" || (tail -n100 build.log; exit 1)
    PREFIX="$XDIR/usr"
    touch "$XDIR/built"
  fi
fi

cd "$ORIGDIR"
echo "toolchain prefix $PREFIX"

make -C "$ARCH" PREFIX=$PREFIX
