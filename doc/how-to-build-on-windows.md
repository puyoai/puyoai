# Windows (cygwin なし) でのビルド方法

## 最初の準備
### 必要なものの package install
次のパッケージをインストールします。基本的に installer が用意されているものです。
- [Visual Studio Express for Desktop](https://www.visualstudio.com/ja-jp/products/visual-studio-express-vs.aspx) 2015 (Microsoft C/C++ Optimizing Compiler Version 19) 以上

- [CMake](https://cmake.org/download/) Ver. 2.8.12 以上
- [Python](https://www.python.org/downloads/) 2.7 系のものを使ってます
- [Git](https://git-for-windows.github.io)

### 必要なものの build &amp; install
[`gflags`](https://github.com/gflags/gflags) と [`glog`](https://github.com/google/glog) については自分で build &amp; install します

```
> git clone https://github.com/gflags/gflags.git
> cd gflags
> mkdir out
> cd out
> cmake .. -G"Visual Studio 14 2015 Win64"
> cmake . --target install --config Release
```

```
> git clone https://github.com/google/glog.git
> cd glog
> mkdir out
> cd out
> cmake .. -G"Visual Studio 14 2015 Win64"
> cmake . --target install --config Release
```

みたいな感じできるはずです。(要確認) 
install できると標準では `C:\Program Files\gflags` と `C:\Program Files\google-glog` ができているはずなので確認してください。

### 環境変数の設定
以下の環境変数を追加してください。 Windows 7 では コントロールパネル→システム→システムの詳細設定→環境変数 で追加できるはずです。
ユーザー環境変数に加えてもシステム環境変数に加えても構いません。

| 変数  |  値            |
|-------|----------------|
|INCLUDE|`C:\Program Files\google-glog\include;C:\Program Files\gflags\Include;C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.10150.0\ucrt;C:\Program Files\Microsoft SDKs\Windows\v7.1\Include`|
|LIB    |`C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10150.0\ucrt\x64`|
|PATH   |`C:\Program Files (x86)\CMake\bin;C:\Program Files (x86)\Python27\;C:\Program Files (x86)\Python27\Scripts;C:\Program Files\Git\cmd;C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin;C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools`|

## Build
git でリポジトリは一通りダウンロードしているものとします。

```
> mkdir out
> cd out
> cmake ..\src -G"Visual Studio 14 2015 Win64"
> cmake --build . --config Release
```

で build されます。なお、`json/json.h` が見つからないという感じのエラーが出る場合がありますが、ビルド順序に依存性がある問題なので
もう一度 `cmake --build . --config Release` するとコンパイルできます。

## 動作
ディレクトリ構成が `foo/bar/Release/hoge.exe` という形になっているので、

```
> duel\Release\duel.exe cpu\sample\Release\sample.exe cpu\sample\Release\sample.exe
```

という形で動きます。(動きません [#214](https://github.com/puyoai/puyoai/issues/214))
出力のリダイレクトなどしたい場合は [`peria/run.bat`](https://github.com/puyoai/puyoai/blob/master/src/cpu/peria/run.bat) を参考に書いてください。

