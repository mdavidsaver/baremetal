#!/bin/sh
set -e -x

die() {
  echo "$1" >&2
  exit 1
}

XREV=6e7c61650a39a67ee02ed58c11d64c94c436bb33

[ "$QDIR" ] || die "QDIR not set"
[ "$XDIR" ] || die "XDIR not set"
[ -e "toolchain.config" ] || die "Missing toolchain.config"

ORIGDIR="$PWD"

cd "$QDIR"

git clone --branch fixirq --depth 10 https://github.com/mdavidsaver/qemu.git src
cd src
HEAD=`git log -n1 --pretty=format:%H`
echo "HEAD revision $HEAD"

[ -e "$QDIR/usr/built" ] && BUILT=`cat $QDIR/usr/built`
echo "Cached revision $BUILT"

if [ "$HEAD" != "$BUILT" ]
then
  echo "Building QEMU"
  git submodule update --init

  rm -rf "$QDIR/usr"
  install -d "$QDIR/usr/build"

  cd "$QDIR/usr/build"

  "$QDIR/src/configure" --prefix="$QDIR/usr" --target-list=arm-softmmu --disable-werror
  make -j2
  make install

  echo "$HEAD" > "$QDIR/usr/built"
fi

rm -rf "$QDIR/src"

ls "$QDIR/usr/bin"

if [ ! -f "$XDIR/$XREV" -o ! -d "$XDIR/usr/bin" ]; then
  echo "Build toolchain"
  rm -rf "$XDIR"
  install -d "$XDIR"
  cd "$XDIR"

  git clone https://github.com/crosstool-ng/crosstool-ng.git
  cd crosstool-ng
  git reset --hard 6e7c61650a39a67ee02ed58c11d64c94c436bb33
  ./bootstrap
  ./configure --enable-local
  make -j2

  cp "$ORIGDIR/toolchain.config" .config
  sed -i -e '/^CT_PREFIX_DIR=/d' .config
  echo "CT_PREFIX_DIR=$XDIR/usr" >> .config

  ./ct-ng build CT_PREFIX="$XDIR/usr" || (tail -n100 build.log; exit 1)

  rm -rf "$XDIR/crosstool-ng"

  touch "$XDIR/$XREV"
else
  echo "Using cached toolchain"
fi

ls "$XDIR/usr/bin"
