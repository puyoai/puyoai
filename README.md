puyoai
======

ぷよ AI を書くためのあれこれです。

## ビルド方法

現状、Mac OS X(10.10 Yosemite) と Ubuntu Linux 14.04 でビルドできることを確認しています。
古めの Linux (Ubuntu 12.04 など)では、SDL 2.0 がないのと、gcc や clang が古いのとで、
いろいろ自分でビルドしなければなりません。

基本的に-std=c++11で書かれています。

### 必要なライブラリ

次のライブラリが必要です。

* google-gflags
* google-glog

GUI を付与したい場合、さらに

* SDL 2.0
* SDL_ttf (2.0)
* SDL_image (2.0)

ビデオキャプチャーをしたい場合、さらに

* gcrypt
* lib-usb1.0

### make のしかた

build ディレクトリを掘って、そこで build するのがオススメです。
というか、必ずそうして下さい。

    $ mkdir build; cd build
    $ cmake ../src
    $ make -j8
    $ make test

* gflags と glog が cmake に発見されなかった場合、それらのライブラリはダミーのものが使われます。
* SDL と SDL_ttf が無い場合、 GUI がつきません。
* capture/ ディレクトリについては、 capture/README を参照してください。

Macの場合、homebrewを使うと楽です。Xcode、コマンドラインツール、homebrewをインストールしてください。デフォルトで/usr/local/includeなどを見ないようになっている可能性がありますが、xcode-select --installを叩いておくと解決します。

次のコマンドで必要なものが入るとおもいます。

    $ brew install pkg-config
    $ brew install gflags glog sdl2 SDL2_ttf SDL2_image ffmpeg libusb libgcrypt

## 実行

    $ cd build
    $ ./duel/duel ./cpu/sample/sample ./cpu/sample/sample

sample は、消せるところがあれば消し、そうでなければ左に積むだけのアルゴリズムで動いています。

## ディレクトリの説明

### src/ ソース

* src/base Mutex とか、noncopyable とか。
* src/core 定数や、ぷよの色の定義など、全員が利用するべきもの。
 * src/core/algorithm AI を実装するときにあると便利なアルゴリズムたち。サーバー実装でも利用。
 * src/core/client クライアントが利用すると便利なもの。
  * src/core/client/ai AIのベース。ここにあるクラスを継承してthink()だけ実装すれば、とりあえず動く。
  * src/core/client/connector サーバーと接続するときに使うと便利なクラス。
 * src/core/field フィールドの実装。gui とかはこれを使っている。
 * src/core/server サーバー実装に必要なもの
  * src/core/srever/connector クライアントとの通信に使うと便利なクラス。
* src/capture キャプチャー関連。画面解析など。
* src/cpu みんなの AI 実装
* src/duel ローカルでの対戦サーバー
* src/gui GUI関連。対戦サーバーやwiiの実装で使う。
* src/third_party 第三者ライブラリをそのまま持ってきたもの。
* src/wii Wii実機と接続して対戦するサーバー。

### その他

* arduino/ Wii実機と接続する際に使う、arduino関連
* data/    フォントとか画像とか
* testdata/ キャプチャー用のテストデータ
* tools/ いろいろなツール類
* deprecated/ 過去のソース。今ほどモジュールに分かれていない。

### そのほか

雑な細かい説明がここに書いてあります。

http://d.hatena.ne.jp/shinichiro_h/20130203

AI のプロトコル仕様について、とても雑で不完全なドキュメントはここにあります。

https://docs.google.com/document/d/1cTIJgRHCBhmxXVtBb45Jm3yAQVaogfVN3oayi-FrA74/edit

適当な情報を wiki に足していく予定です…

https://github.com/puyoai/puyoai/wiki
