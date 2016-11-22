# `core/pattern` 概説

## 基本的な使い方

`core/pattern` ではパターンマッチングのライブラリを提供しています。
大きく分けて `DecisionBook` と `PatternBook` の 2 種類があり、
`DecisionBook` はフィールドの状態とツモを元に具体的な操作を教えてくれるデータベース、
`PatternBook` はフィールドの状態からぷよを消さずに作れるフィールドの形を教えてくれるデータベース
と考えてもらって構いません。

基本的にはそれぞれのデータを記述したファイルを用意しておいて
```
// DecisionBook
DecisionBook book;
book.load("decision_book.toml"); // 全ゲーム中1回loadすれば大丈夫
Decision decision = book.nextDecision(field, seq);
```
```
// PatternBook
PatternBook book;
book.load("pattern_book.toml"); // 全ゲーム中1回loadすれば大丈夫
auto callback = [&](CoreField&&, const ColumnPuyoList&, int, const FieldBits&, const PatternBookField&) {
  ... // ここで評価する
};
book.complement(field, callback);
```

という感じに使います。
特に `PatternBook` のサンプル内に出てきた `callback` は、与えられたフィールドからぷよを消さずに組み立てられる
パターン全てについて呼び出されます。
