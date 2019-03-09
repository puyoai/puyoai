#!/bin/bash

basedir="$(cd $(dirname "$0")/..; pwd)"
rootdir="$basedir/puyoai"
outdir="$rootdir/out/Debug"

# build
cd $rootdir
gn gen out/Debug
ninja -C out/Debug

# run tests
./build/run_unittest.py
