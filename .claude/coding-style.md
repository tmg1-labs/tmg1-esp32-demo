# コーディング規約

## 命名規則
- 変数・関数: camelCase（例: `riceMode`, `findOptimalRiceK`）
- クラス・型: PascalCase（例: `Tmg1Stream`, `Decoder`, `FileHeader`）
- 定数・マクロ: UPPER_SNAKE_CASE（例: `RICE_MODE_PER_FRAME`, `PER_FRAME_K`）
- メンバ変数: 先頭アンダースコア（例: `_low`, `_range`, `_frameBufferSize`）
- 名前空間: `tmg1`

## フォーマッタ / Linter
- clang-format: `.clang-format`（BasedOnStyle: Google, ColumnLimit: 120）

## コメント・ドキュメント
- ソースコードには**日本語のコメント**を記入し、仕組みを把握しやすくする。
- アルゴリズムの不変条件（Range コーダの正規化条件など）はコメントで明示する。
- README はライブラリの使い方を**英語**で書き、**日本語版**も同時に用意する。

## その他のルール
- 軽量・高速・低メモリを意識する（ESP32 ターゲット）。
- 入出力はストリーム（`Tmg1Stream`）経由。フレーム全体をメモリに溜めない
  （`tmg1-codec` が提供するAPIを`src/main.cpp`から利用する側であり、本リポジトリ自身は
  ストリーム抽象を実装しない）。
- Arduino 専用コードは `#if defined(ARDUINO)`、デスクトップ専用は `#if !defined(ARDUINO)` でガード。
- 本リポジトリは `tmg1-codec` の C API を直接変更しない（変更は `tmg1-codec` 本流で行う。
  引数同期ルールは同リポジトリの coding-style.md 参照）。
- `test/` の PlatformIO native テストは `src/main.cpp` 側のロジック（表示・LittleFS読み込み等）を
  対象とする。codec自体のテストは `tmg1-codec` の CI が担う。
