#!/bin/bash

BIN_PATH=dpkg/usr/bin

mkdir -p $BIN_PATH

cp third_party/pear/third_party/libnice/builddir/stun/tools/stund $BIN_PATH/
cp cmake/src/berry-share $BIN_PATH/

dpkg-deb -b dpkg berry-share.deb


