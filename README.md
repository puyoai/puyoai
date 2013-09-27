puyoai
======

ぷよ AI を書くためのあれこれです。

### ビルド方法

    $ cmake .
    $ make

* 適当なディレクトリを掘ってから build することもできます。ただし、現状実行スクリプトがコピーされません。

    $ mkdir build
    $ cd build
    $ cmake .. 
    $ make

* gflags と glog が cmake に発見されなかった場合、それらのライブラリはダミーのものが使われます。
* SDL と SDL_ttf が無い場合、 GUI がつきません。
* capture/ ディレクトリについては、 capture/README を参照ください。

### 実行

    $ ./duel/duel ./cpu/sample/sample ./cpu/sample/sample

### 説明

雑な細かい説明がここに書いてあります。

http://d.hatena.ne.jp/shinichiro_h/20130203

AI のプロトコル仕様について、とても雑で不完全なドキュメントはここにあります。

https://docs.google.com/document/d/1cTIJgRHCBhmxXVtBb45Jm3yAQVaogfVN3oayi-FrA74/edit

適当な情報を wiki に足していく予定です…

https://github.com/puyoai/puyoai/wiki
