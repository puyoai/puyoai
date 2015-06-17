# cygwin でのビルド方法

* original author: @__roxion1377
* edited by: @mayah_puyo

## cygwin 上のパッケージインストール

次のパッケージをインストール
* gcc-core
* gcc-g++
* git
* cmake
* make
* autoconf
* automake
* pkg-config
* libSDL2_2.0_0
* libSDL2-devel
* libSDL2_ttf2.0_0
* libSDL2_ttf-devel
* libSDL2_image2.0_0
* libSDL2_image-devel
をインストール

## glog のビルド・インストール

    $ git clone https://github.com/google/glog
    $ cd glog

src/utilities.ccの267行目の
    #elif defined OS_WINDOWS || defined OS_CYGWIN
を
    #elif defined OS_WINDOWS

src/googletest.hの524行目と557行目の
    #if defined(OS_WINDOWS) || defined(OS_CYGWIN)
を
    #if defined(OS_WINDOWS)
にして

    $ ./configure --prefix=/usr
    $ make -j4
    $ make install

## gflags のビルド・インストール

    $ git clone https://github.com/gflags/gflags
    $ cd gflags
    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=/usr ../
    $ make -j4
    $ make install

## puyoai のビルド

    $ git clone --depth 1 https://github.com/puyoai/puyoai
    $ mkdir -p out/Default
    $ CMAKE_LEGACY_CYGWIN_WIN32=1 cmake ../../src
    $ make -j4
    $ ./duel/duel.exe --use_cui=true --use_gui=false cpu/mayah/run_fast.sh cpu/test_lockit/rendaS9.sh

NOTE:
cpu/mayah/run.sh is not working as the author intended.
Try cpu/mayah/run_fast.sh instead of cpu/mayah/run.sh
