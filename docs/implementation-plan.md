# TMG1 コーデック 実装計画

作成日: 2026-06-03

---

## 背景と方針転換の経緯

### 現状
- `tmg1-encoder-dotnet`: C# によるエンコーダ（PCツール）
- `tmg1-arduino`: C++ による ESP32 向けデコーダ

### 課題
- 同一アルゴリズムが C# と C++ に重複実装されている
- .NET CLIはプラットフォーム依存が強く、将来性に乏しい
- Rangeコーダが64bit _range を使用しており、128bit中間演算が必要 → MSVC非対応・ESP32で低速

### 方針
1. **共通C++コーデックライブラリ** `tmg1-codec` を新規作成
2. **Rust CLIエンコーダ** `tmg1-rust-cli` を新規作成（.NETエンコーダを終息）
3. **Arduino側** をサブモジュール化（既存デコーダを共通ライブラリに置き換え）
4. **TMG1フォーマットをv2へメジャーバージョンアップ**（32bit Rangeコーダへの変更を含む）

---

## TMG1 v2 フォーマット変更点

### Rangeコーダの32bit化

| 項目 | v1 | v2 |
|---|---|---|
| `_range` 型 | uint64 | uint32 |
| `_low` 型 | uint64 | uint64 |
| `_code` 型 | uint64 | uint32 |
| 初期 `_range` | `0xFFFFFFFFFFFFFFFF` | `0xFFFFFFFF` |
| `BottomValue` | `1 << 24` | `1 << 23` |
| `TopValue` | `1 << 56` | `1 << 24` |
| 初期読み込みバイト数 | 8 | 4 |
| 中間演算 | 最大 2^84 → **uint128必須** | 最大 2^52 → **uint64で足りる** |

**32bit化の効果:**
- ESP32（32bitMCU）でのRange演算が大幅高速化
- uint128カスタム実装が不要になりコードが単純化
- MSVC環境でもビルド可能になる

ファイルヘッダの Version フィールド（Byte 4）を `2` にすることで識別する。
v1ファイルのデコードはv2デコーダでは対応しない（試作段階のため互換性は不要と判断）。

---

## 新リポジトリ: `tmg1-codec`

### 目的
エンコーダ・デコーダの両方を含む C++ コーデックライブラリ。  
Arduino と Rust CLI の両方がサブモジュールとして取り込む。

### ディレクトリ構成

```
tmg1-codec/
├── include/
│   └── tmg1/
│       ├── types.h           # 定数・enum・共通構造体
│       ├── io.h              # I/Oコールバック定義
│       ├── rice_reader.h
│       ├── rice_writer.h
│       ├── range_decoder.h   # 32bit版
│       ├── range_encoder.h   # 32bit版
│       ├── freq_model.h
│       ├── prediction.h
│       ├── decoder.h
│       └── encoder.h
├── src/
│   ├── rice_reader.cpp
│   ├── rice_writer.cpp
│   ├── range_decoder.cpp
│   ├── range_encoder.cpp
│   ├── freq_model.cpp
│   ├── prediction.cpp
│   ├── decoder.cpp
│   └── encoder.cpp
├── c_api/
│   ├── tmg1_c.h             # extern "C" API（Rust FFI用）
│   └── tmg1_c.cpp
├── test/
│   ├── test_rice.cpp
│   ├── test_range.cpp
│   ├── test_prediction.cpp
│   ├── test_codec.cpp        # エンコード→デコードのラウンドトリップ
│   └── vectors/             # テストベクタ（バイナリファイル）
├── CMakeLists.txt           # デスクトップビルド・テスト用
└── library.properties       # Arduino ライブラリ認識用
```

### I/O抽象化設計

Arduino の `Stream` と デスクトップ（Rust FFI含む）の両方に対応するため、  
**コールバックベースのI/F** を採用する。

```c
// include/tmg1/io.h
typedef int  (*Tmg1ReadFn) (void* ctx, uint8_t* buf, size_t len);
typedef int  (*Tmg1WriteFn)(void* ctx, const uint8_t* buf, size_t len);
typedef long (*Tmg1TellFn) (void* ctx);
typedef int  (*Tmg1SeekFn) (void* ctx, long offset, int origin);

typedef struct {
    void*        ctx;
    Tmg1ReadFn   read;
    Tmg1WriteFn  write;
    Tmg1TellFn   tell;   // NULL可（シーク不要な場合）
    Tmg1SeekFn   seek;   // NULL可
} Tmg1Stream;
```

Arduino 側では `Stream` を `Tmg1Stream` にラップするアダプタを提供する。

### Rangeコーダ（v2 32bit版）設計

```cpp
// 中間演算はすべて uint64 で完結する
// scaledCode = ((code - low + 1) * total - 1) / range
// (code - low + 1) <= range <= 2^32, total <= 2^20
// 積 <= 2^52 → uint64 に収まる

static const uint32_t BottomValue = 1u << 23;
static const uint32_t TopValue    = 1u << 24;

uint32_t _low;   // 正規化後は [0, 2^32) で wrap
uint32_t _range;
uint32_t _code;
```

### C API（Rust FFI用）

```c
// c_api/tmg1_c.h
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tmg1Decoder Tmg1Decoder;
typedef struct Tmg1Encoder Tmg1Encoder;

Tmg1Decoder* tmg1_decoder_create(Tmg1Stream* stream);
void         tmg1_decoder_destroy(Tmg1Decoder* dec);
int          tmg1_decoder_read_frame(Tmg1Decoder* dec, uint8_t* out, size_t out_size);

Tmg1Encoder* tmg1_encoder_create(Tmg1Stream* stream, const Tmg1EncodeConfig* config);
void         tmg1_encoder_destroy(Tmg1Encoder* enc);
int          tmg1_encoder_write_frame(Tmg1Encoder* enc, const uint8_t* frame, size_t size);
int          tmg1_encoder_finish(Tmg1Encoder* enc);

#ifdef __cplusplus
}
#endif
```

---

## 新リポジトリ: `tmg1-rust-cli`

### 目的
PC 向けエンコード CLI ツール。`tmg1-codec` を FFI 経由で使用。

### 構成

```
tmg1-rust-cli/
├── vendor/
│   └── tmg1-codec/          # gitサブモジュール
├── build.rs                 # tmg1-codec の CMake ビルドを呼び出す
├── src/
│   ├── main.rs
│   ├── ffi.rs               # tmg1_c.h のバインディング（bindgen使用）
│   └── io_adapter.rs        # Rust の Read/Write を Tmg1Stream に変換
└── Cargo.toml
```

### FFI接続方針
- `bindgen` で `tmg1_c.h` から Rust バインディングを自動生成
- `build.rs` で `cmake` クレートを使って `tmg1-codec` をスタティックリンク
- `unsafe` ブロックは `ffi.rs` に閉じ込め、外部には安全なラッパーを公開

---

## 既存リポジトリ更新: `tmg1-arduino`

### 変更内容
- `src/` 内の既存実装（`Tmg1Decoder.cpp`, `RangeDecoder.h` 等）を削除
- `lib/tmg1-codec` としてサブモジュール追加
- `src/main.cpp` のサンプルプログラムを新APIに合わせて更新

### Arduino 向け Stream アダプタ

```cpp
// tmg1-codec 内に提供予定
#ifdef ARDUINO
#include <Stream.h>
Tmg1Stream tmg1_stream_from_arduino(Stream& s);
#endif
```

---

## .NETエンコーダ (`tmg1-encoder-dotnet`)

- `tmg1-rust-cli` 完成後に **アーカイブ（終息）**
- v2フォーマット対応は行わない

---

## 実装順序

### Phase 1: tmg1-codec コアライブラリ

1. リポジトリ作成・CMake/library.properties 設定
2. `types.h` / `io.h` 設計・実装
3. Rice コーダ移植（reader/writer）
4. 頻度モデル移植
5. **Rangeコーダ 32bit版 実装**（v1コードを参考に再設計）
6. 予測フィルタ移植
7. デコーダ実装
8. エンコーダ実装
9. C APIラッパー実装
10. ラウンドトリップテスト・テストベクタ整備

### Phase 2: tmg1-arduino 更新

1. サブモジュール追加
2. 既存実装削除・新APIに差し替え
3. サンプルプログラム更新・動作確認

### Phase 3: tmg1-rust-cli 新規作成

1. リポジトリ作成・サブモジュール追加
2. build.rs / bindgen 設定
3. I/Oアダプタ実装
4. CLI コマンド実装（encode / decode / info）
5. 動作確認・.NETエンコーダとのクロスチェック（v1ファイルで）

### Phase 4: クリーンアップ

1. tmg1-encoder-dotnet をアーカイブ
2. 各リポジトリの README 更新

---

## テスト戦略

- **ユニットテスト**: 各コンポーネント（Rice/Range/FreqModel/Prediction）を独立にテスト
- **ラウンドトリップテスト**: エンコード→デコードで元データと一致することを確認
- **テストベクタ**: `test/vectors/` に参照バイナリを置き、クロス実装間の互換性を担保
- **GitLab CI**: Phase 1 完了時点でパイプライン設定（Unity テストフレームワーク）
- **カバレッジ**: 100%に近づけることを目標とする（CLAUDE.md 方針に従う）
