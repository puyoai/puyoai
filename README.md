# puyoai

[![Build Status](https://circleci.com/gh/puyoai/puyoai.png?circle-token=:circle-token)](https://circleci.com/gh/puyoai)

ぷよぷよ通のAIを書くためのフレームワークです。

実際に動いているAIの例: http://www.nicovideo.jp/watch/sm26167419

## 前提環境

### OS

現状、Mac OS X と Ubuntu、Windows でビルドできることを確認しています。
また、32bit環境では全くテストしていません。

### コンパイラ

C++14 がサポートされているコンパイラを使う必要があります。
Mac OSX や Linux では clang もしくは gcc を、Windows では Visual C++ を使うことが想定されています。

### ハードウェア (CPU)

高速化のため、CPU に AVX 命令が載っていることを前提にしている箇所があります。
すなわち、2011 年〜 2012 年以降の CPU のみを対象にしています。
現状は AVX2 は前提にしていませんが、AVX2が使える場合はAVX2対応のコードがあり、AVX1版と比べて1.5倍〜2倍高速です。ただし、AVX2版のコードはgcc-4.7では全く性能がでないことが分かっているため、clangを使ってください。より新しいgccであれば直っているかもしれませんが、確認していません。

## ビルド方法

### 必要なライブラリのインストール

#### 全プラットフォーム共通

[depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
  をインストールしてください。`PATH` を通すのを忘れないように。

#### Linux

```shell
$ sudo apt-get install git clang
$ sudo apt-get install libprotobuf-dev libcurl4-nss-dev
$ sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
$ sudo apt-get install libmicrohttpd-dev libffms2-dev libusb-1.0-0-dev
```

#### Mac (homebrew 前提)

Xcode、コマンドラインツール、homebrew をインストールしてください。
デフォルトで `/usr/local/include` などを見ないようになっている可能性がありますが、`xcode-select --install` を叩いておくと解決するはずです。

```shell
$ brew install pkg-config
$ brew install sdl2 SDL2_ttf SDL2_image ffmpeg libusb protobuf
```

#### Windows

[Visual Studio 2017](https://visualstudio.microsoft.com/) をインストールしてください。

また、他プラットフォームと同様にビルドツールとして ninja を使いたい場合は VC 関係の設定を行うため以下の bat を各コマンドプロンプトで実行してください。

```
> %ProgramFiles%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat
```

### レポジトリのダウンロード

1. ターミナル、もしくはコマンドプロンプトで、適当なディレクトリ (e.g. `~/repos/puyoai`) を作って移動し、そこで `gclient config --unmanaged https://github.com/puyoai/puyoai` コマンドを入力してください。`.gclient` ファイルがそのディレクトリに作られます。
1. `gclient sync` コマンドを入力してください。必要なファイルなどがダウンロードされます。

For contributors note: puyoai レポジトリに push できる権限がある場合、`gclient config` は、次のようにしてください。

```shell
$ gclient config --unmanaged git@github.com:puyoai/puyoai
```

### 必要なライブラリの説明

GUI を付与したい場合、さらに次のライブラリが必要です。

* [SDL 2.0](https://www.libsdl.org/index.php)
* [SDL_ttf (2.0)](https://www.libsdl.org/projects/SDL_ttf/)
* [SDL_image (2.0)](https://www.libsdl.org/projects/SDL_image/)

http サーバーを利用して、GUIを付与することもできます。この場合、次のライブラリが必要です。

* microhttpd

実機でAIを動かすなど、ビデオキャプチャーが必要な場合、さらに次のライブラリが必要です。

* lib-usb1.0

### ビルドのしかた Linux/Mac

```shell
$ cd ~/repos/puyoai/puyoai
$ gn gen --args="is_debug=false" out/Release
$ ninja -C out/Release
```

ただしこのままだと SDL が必要なものはビルドされません。

```shell
$ gn args out/Release
```

とするとエディタが立ち上がるので、

```
is_debug = false
use_capture = true
use_usb = true
use_gui = true
use_httpd = true
use_libcurl = true
use_tcp = true
use_curl = true
```

とすると、全部入りになります (2019-02-23 現在)。どのようなオプションがあるかは、[build/BUILDCONFIG.gn](build/BUILDCONFIG.gn) の `declare_args` 内（複数あります)を参照してください。

`is_debug` を `true` にすると、デバッグビルドになります。

### ビルドのしかた Windows

```
> cd %HOME%\repos\puyoai\puyoai
> gn gen --args="is_debug=false" out/Release
> ninja -C out/Release
```

Linux/Mac で使えるコンフィグ option のうちいくつかは動きません。

もし Visual Studio の solution file を生成したい場合、

```
> cd %HOME%\repos\puyoai\puyoai
> gn gen --args="is_debug=false" --ide=vs out/Release
```

とすると、`all.sln` ファイルが `out/Release` 以下に生成されます。

### How to run test

ビルドした後、

```shell
$ cd ~/repos/puyoai/puyoai
$ python build/run_unittest.py --build-dir=out/Debug
```

で全テスト走ります。

### エラーがでた

もしかしたら、master ブランチが壊れているかもしれません。この場合、気づいたメンバーによってすぐに修復されます。
このドキュメントの先頭に貼ってある[CircleCI](https://circleci.com/gh/puyoai)のバッジが `PASSED` になっていなければ、現状のコードは壊れています。
Issue List に問題を報告、もしくは Pull request を送ってください。

## 実行

```shell
$ cd out/Release
$ ./duel ./cpu/sample/sample ./cpu/sample_rensa/sample_rensa
```

`duel` は対戦サーバで、筐体のような役割を果たします。1 つ目の引数 `sample` は 1P 側を担当する AI、2 つ目の引数 `sample_rensa` は 2P 側を担当する AI です。
`duel` にオプションを渡すことで対戦速度を上げたり、ぷよの色を指定できたり、といろんな機能を引き出すことができます。ドキュメント化されていないものもありますが、

```shell
$ ./duel --help
```

とすると実装されている機能と必要なオプションが全部出てきます。とりあえずはそちらを参照してください。

## ディレクトリの説明

* doc/ ドキュメント
* src/ 主なプログラムのソース。
* build/ ビルド関連のスクリプトなど。意味のわかる方のみ使ってください。
* arduino/ Wii 実機やアーケード基板と接続する際に使う、arduino関連のソース。
* data/    フォントとか画像とか。
* testdata/ キャプチャー用のテストデータ。
* tools/ いろいろなツール類。なぜ src/ 以下じゃないのか？
