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
- 入出力はストリーム（`Tmg1Stream`）経由。フレーム全体をメモリに溜めない。
- Arduino 専用コードは `#if defined(ARDUINO)`、デスクトップ専用は `#if !defined(ARDUINO)` でガード。
- C API（`c_api/`）の引数は C++ 側の `EncodeConfig` と同期させる（riceMode/riceK 等）。
- テストはカバレッジ 100% に近づける。FileHeader 等の構造変更時はテストデータも更新する。
