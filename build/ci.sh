#!/bin/bash

basedir="$(cd $(dirname "$0")/..; pwd)"
rootdir="$basedir/puyoai"
outdir="$rootdir/out/Debug"

export PATH=${rootdir}/depot_tools:${PATH}

gclient config --spec 'solutions = [
  {
    "url": "https://github.com/puyoai/puyoai.git",
    "managed": False,
    "name": "puyoai",
    "deps_file": "DEPS",
    "custom_deps": {
      "test_resources": None,
    },
  },
]
'
gclient sync

# build
cd $rootdir
gn gen out/Debug
ninja -C out/Debug

# run tests
./build/run_unittest.py
