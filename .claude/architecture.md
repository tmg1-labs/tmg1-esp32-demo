# アーキテクチャ方針

## 全体構成

TMG1 動画フォーマットのデコーダ/エンコーダ。コアアルゴリズムは
プラットフォーム非依存の C++ ライブラリ `lib/tmg1-codec` に集約し、
Arduino(ESP32) と Rust CLI(`tmg1-cli`) の双方から共有する。

```
tmg1-esp32-demo (このリポジトリ)
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
- **Range コーダを v2 化（64bit）**: MSVC 対応・`__uint128_t` 実装回避。
  `_low`/`_range`/`_code` はすべて uint64（`_range` 初期値 `0xFFFF…FFFF`）、
  `TopValue=1<<56` / `BottomValue=1<<24`。divide-first（`step=_range/total` 先算）で積が uint64 に収まり uint128 不要。
  フラッシュは `_low >> 56` を 8 バイト出力。v1 互換は維持しない（試作段階のため）。
- **.NET エンコーダは終息** → Rust CLI に置き換え（プラットフォーム依存・将来性）。
- **コーデックをサブモジュール化**: CLI/Arduino で単一の codec コミットを共有する。

## Range コーダ v2 実装の要点（壊しやすい不変条件）

- 共通演算（divide-first）: `step = _range / total` を先に求め、`_low += step * cumulative; _range = step * symbolFreq`（`_range==0` なら 1）。
- デコーダ: `scaledCode = (_code - _low) / step`（`scaledCode >= total` なら `total-1` にクランプ）。
- 正規化（while ループ）: `sum=_low+_range` のオーバーフローを検出し
  `xorVal = overflow ? 0xFFFF…FFFF : (_low ^ sum)`。
  `xorVal >= TopValue(1<<56) && _range >= BottomValue(1<<24)` で break、
  それ以外は 1 バイト処理（`_low<<=8; _range<<=8`、デコーダは併せて `_code = (_code<<8)|readByte()`）。
- 初期化: 8 バイト読み `_code = (_code<<8)|byte` を 8 回。
- エンコーダ flush: `_low >> 56` を 8 回出力。

## 禁止パターン

- v1 フォーマットとの後方互換を持ち込まない（v2 のみ）。
- `src/` を native ビルドに含めない（Arduino 依存があるため `build_src_filter = -<*>`）。
- codec の分岐コピーを増やさない。修正は `lib/tmg1-codec` 本流に入れ、サブモジュールポインタで同期する。
- `std::max/std::min` に整数リテラルを直接渡さない（RISC-V で型不一致。`(uint32_t)` 明示）。
