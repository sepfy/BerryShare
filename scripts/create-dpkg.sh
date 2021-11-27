#!/bin/bash

VERSION=0.`git log --pretty=oneline | wc -l`.0
BIN_PATH=dpkg/usr/bin
LIB_PATH=dpkg/usr/lib

mkdir -p $BIN_PATH $LIB_PATH

cp third_party/pear/third_party/libnice/builddir/stun/tools/stund $BIN_PATH/
cp cmake/src/berry-share $BIN_PATH/
cp third_party/pear/cmake/src/libpear.so $LIB_PATH

sed -i "/Version/c\Version: $VERSION" dpkg/DEBIAN/control

dpkg-deb -b dpkg berry-share_$VERSION.deb


