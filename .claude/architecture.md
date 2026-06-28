# アーキテクチャ方針

## 全体構成

TMG1 動画フォーマットのデコーダ/エンコーダ。コアアルゴリズムは
プラットフォーム非依存の C++ ライブラリ `lib/tmg1-codec` に集約し、
Arduino(ESP32) と Rust CLI(`tmg1-cli`) の双方から共有する。

```
tmg1-arduino (このリポジトリ)
├── src/main.cpp            ... ESP32 サンプル (LittleFS から再生 → U8g2 OLED 表示)
├── lib/tmg1-codec/         ... 共通コーデック (サブモジュール / 別リポジトリ)
│   ├── include/tmg1/       ... ヘッダ (types.h, io.h, decoder.h, encoder.h ...)
│   ├── src/                ... 実装
│   ├── c_api/              ... extern "C" API (Rust FFI 用)
│   └── test/               ... Unity テスト (CMake 経由)
├── test/                   ... PlatformIO native テスト
└── docs/                   ... 実装計画・仕様サブモジュール
```

## 入出力モデル

- **ストリーム抽象 `Tmg1Stream`**: コールバックベースの I/O 抽象。
  Arduino の `Stream`、デスクトップの `FILE*`、メモリバッファのいずれにも対応。
- メモリ消費を抑えるため、入力・出力ともにストリーム経由でフレーム単位に処理する。

## 技術選定の理由

- **共通ライブラリを C++ に**: Arduino はネイティブ C++、Rust は FFI で接続可能。
- **Range コーダを v2 で 32bit 化**: ESP32 高速化・MSVC 対応・uint128 実装回避。
  `_low`/`_range`/`_code` はすべて uint32、フラッシュは 4 バイト。
  v1 互換は維持しない（試作段階のため）。
- **.NET エンコーダは終息** → Rust CLI に置き換え（プラットフォーム依存・将来性）。
- **コーデックをサブモジュール化**: CLI/Arduino で単一の codec コミットを共有する。

## Range コーダ v2 実装の要点（壊しやすい不変条件）

- デコーダ: `scaledCode = ((code - low + 1) * total - 1) / range` を uint64 で計算（積 ≦ 2^52）。
- 正規化条件: `(_low ^ (_low + _range)) < (1u << 24) || _range < (1u << 23)`。
- 初期化: 4 バイト読み `_code = (b3<<24)|(b2<<16)|(b1<<8)|b0`。
- エンコーダ flush: `_low >> 24` を 4 回出力。

## 禁止パターン

- v1 フォーマットとの後方互換を持ち込まない（v2 のみ）。
- `src/` を native ビルドに含めない（Arduino 依存があるため `build_src_filter = -<*>`）。
- codec の分岐コピーを増やさない。修正は `lib/tmg1-codec` 本流に入れ、サブモジュールポインタで同期する。
- `std::max/std::min` に整数リテラルを直接渡さない（RISC-V で型不一致。`(uint32_t)` 明示）。
