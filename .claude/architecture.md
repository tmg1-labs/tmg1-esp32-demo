# アーキテクチャ方針

## 全体構成

TMG1 動画フォーマットのデコーダ/エンコーダ。コアアルゴリズムは
プラットフォーム非依存の C++ ライブラリ `tmg1-codec`（別リポジトリ）に集約し、
Arduino(ESP32) と Rust CLI(`tmg1-cli`) の双方から共有する。本リポジトリはそれを
PlatformIO の `lib_deps`（git タグ `#v0.2.0`）で取得して実機で動かす消費者。

```
tmg1-esp32-demo (このリポジトリ)
├── src/main.cpp            ... ESP32 サンプル (LittleFS から再生 → U8g2 OLED 表示)
├── test/                   ... PlatformIO native テスト
└── docs/                   ... 実装計画

tmg1-codec (別リポジトリ / lib_deps で取得 → .pio/libdeps/<env>/tmg1-codec)
├── include/tmg1/           ... ヘッダ (types.h, io.h, decoder.h, encoder.h ...)
├── src/                    ... 実装
├── c_api/                  ... extern "C" API (Rust FFI 用)
└── test/                   ... Unity テスト (codec 本体 CI の CMake で実行)
```

## 入出力モデル

- **ストリーム抽象 `Tmg1Stream`**: コールバックベースの I/O 抽象。
  Arduino の `Stream`、デスクトップの `FILE*`、メモリバッファのいずれにも対応。
- メモリ消費を抑えるため、入力・出力ともにストリーム経由でフレーム単位に処理する。

## 技術選定の理由

- **共通ライブラリを C++ に**: Arduino はネイティブ C++、Rust は FFI で接続可能。
- **Range コーダを v2 化（64bit）**: MSVC 対応・`__uint128_t` 実装回避。
  実装詳細・不変条件は `tmg1-codec` 本体の `.claude/architecture.md` を参照
  （本リポジトリはコーデックの中身に立ち入らない消費者のため、詳細は転記しない）。
- **.NET エンコーダは終息** → Rust CLI に置き換え（プラットフォーム依存・将来性）。
- **コーデックを外部ライブラリ化**: CLI/Arduino で単一の codec タグ（コミット）を共有する。

## 禁止パターン

- v1 フォーマットとの後方互換を持ち込まない（v2 のみ）。
- `src/` を native ビルドに含めない（Arduino 依存があるため `build_src_filter = -<*>`）。
- codec の分岐コピーを増やさない。修正は `tmg1-codec` 本流リポジトリに入れ、タグを切って `lib_deps` の `#vX.Y.Z` を上げて同期する。
