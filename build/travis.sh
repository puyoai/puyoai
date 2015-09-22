#!/bin/bash

rootdir="$(cd $(dirname "$0")/..; pwd)"
outdir="$rootdir/out/Debug+Asan"
srcdir="$rootdir/src"

declare -a cmake_opts
cmake_opts+=("-GNinja")
cmake_opts+=("-DCMAKE_C_COMPILER=clang")
cmake_opts+=("-DCMAKE_CXX_COMPILER=clang++")
cmake_opts+=("-DCMAKE_BUILD_TYPE=Debug")

declare -a cmake_cxx_flags
cmake_cxx_flags+=("-std=c++11" "-Werror" "-Wall" "-Wextra")
cmake_cxx_flags+=("-Wno-error=unused-variable" "-Wno-error=unused-parameter")
cmake_cxx_flags+=("-Wno-error=missing-field-initializers")
cmake_cxx_flags+=("-fsanitize=address")
cmake_cxx_flags+=("-O2")
cmake_cxx_flags+=("-march=native")

cmake_opts+=("-DCMAKE_CXX_FLAGS=${cmake_cxx_flags[*]}")

mkdir -p $outdir
cd $outdir
cmake "${cmake_opts[@]}" $srcdir

ninja 
ninja test
