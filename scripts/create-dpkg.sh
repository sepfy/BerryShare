#!/bin/bash

VERSION=0.`git log --pretty=oneline | wc -l`.0
BIN_PATH=dpkg/usr/bin

mkdir -p $BIN_PATH

cp third_party/pear/third_party/libnice/builddir/stun/tools/stund $BIN_PATH/
cp cmake/src/berry-share $BIN_PATH/

sed -i "/Version/c\Version: $VERSION" dpkg/DEBIAN/control

dpkg-deb -b dpkg berry-share_$VERSION.deb


