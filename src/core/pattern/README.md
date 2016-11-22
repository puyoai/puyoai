# `core/pattern` 概説

`core/pattern` ディレクトリではパターンマッチングのライブラリを提供しています。

TODO: `field` などの基本フォーマットの説明

## `DecisionBook`
`DecisionBook` は序盤の定積の定義に使います。
toml ファイルに定積を定義することで、フィールドの形とツモから操作を決定してくれます。

### Format
```
[[book]]
field = [
  "..A...",
  "..A...",
]
AA   = [3, 0]
AAAA = [5, 2]
```

`[[book]]` に挟まれた部分が 1 つの定積を表します。定積の定義には `field` とツモ情報を与え、
「このフィールド状態でこのツモだったらこう操作する」という情報を書きます。

### API
基本的には DecisionBook を読み込んだ後で
```
Decision DecisionBook::nextDecision(const CoreField& field, const KumipuyoSeq& sequence) const;
```
を呼び出すと、`field` と `sequence` が一致する定積があればその後の操作を返してくれます。

--------

## `PatternBook`
`PatternBook` は積んでいく形のパターンを定義します。
toml ファイルに定積を定義することで、フィールドの形とツモから操作を決定してくれます。

与えられたフィールドにあるぷよの形と定積の形がミスマッチしていなければ、定積にある形で
補間した形のフィールドをコールバック関数に与えてくれます。

### Format
```
[[pattern]]
field = [
  ".Cb...",
  "*CB...",
  "CBA...",
  "CBA...",
  "@BAA..",
]
name = "Kaidan"
ignition = 1
score = 2.424
precondition = [[1, 2], [1, 3]]
```
#### `field`, `not_field`
フィールドの形です。大文字のアルファベットで基本的な形を定義し、小文字のアルファベットで小さな変更を許容します。
`&` で鉄ぷよ(TODO:説明追加)、`*` で任意のぷよを表現します。
`field` は必須項目、`not_field` は(TODO:説明追加)

#### `name`
定積の名前です。

#### `ignition`
TODO: 説明追加。ソース読んでるだけだと使い道が分からない。

#### `score`
評価値として使いたい点数

#### `precondition`
このマスのぷよが一致してない場合は他がどうであろうと不一致扱いになります。

### API
基本的 API は
```
void PatternBook::complement(const CoreField& field, const ComplementCallback& callback) const;
```
で、与えられたフィールド `field` にあるぷよの形とマッチする定積ついて、足りない部分のぷよを補間した上で
`callback` を呼び出してくれます。
