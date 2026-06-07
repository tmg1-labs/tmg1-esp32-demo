# MEMORY.md

## セッション履歴

---

### 2026-06-07 セッション 1

#### 作業内容
TMG1コーデックのアーキテクチャ見直しと実装計画策定。

#### 完了したこと
- 両コードベース（tmg1-arduino / tmg1-encoder-dotnet）の重複アルゴリズム分析
- 共通化アーキテクチャの設計方針決定
- `docs/implementation-plan.md` 作成・コミット・プッシュ（feature/codec-separation）

#### 決定事項

| 決定 | 内容 | 理由 |
|---|---|---|
| 共通ライブラリ言語 | C++ | ArduinoはネイティブC++、RustはFFIで接続可能 |
| リポジトリ名 | `tmg1-codec` | |
| .NETエンコーダ | 終息（Rust CLIに置き換え） | プラットフォーム依存・将来性 |
| Rangeコーダ32bit化 | v2フォーマットとして採用 | ESP32高速化・MSVC対応・uint128実装不要 |
| v1互換性 | 維持しない | 試作段階のため |
| esp-rs | 様子見（Arduino公式対応まで） | esp-idf必須で面倒 |
| I/O抽象化 | コールバックベース（Tmg1Stream） | Arduino・デスクトップ両対応 |
| Rust FFI | bindgen + cmakeクレート | 自動バインディング生成 |

---

### 2026-06-07 セッション 2

#### 作業内容
Phase 1: `lib/tmg1-codec` 共通C++ライブラリの実装。

#### 完了したこと

| ファイル | 内容 |
|---|---|
| `lib/tmg1-codec/include/tmg1/types.h` | 定数・enum・FileHeader・FrameHeader・EncodeConfig |
| `lib/tmg1-codec/include/tmg1/io.h` | Tmg1Stream コールバック定義、メモリバッファヘルパー |
| `lib/tmg1-codec/include/tmg1/freq_model.h` | 適応型頻度モデル (Arduino依存なし) |
| `lib/tmg1-codec/include/tmg1/rice_reader.h/.cpp` | Riceビット読み取り (Arduino依存なし) |
| `lib/tmg1-codec/include/tmg1/rice_writer.h/.cpp` | Riceビット書き込み (Tmg1Stream経由) |
| `lib/tmg1-codec/include/tmg1/range_decoder.h/.cpp` | **v2 32bit Rangeデコーダ** (IRAM_ATTR対応, uint128不要) |
| `lib/tmg1-codec/include/tmg1/range_encoder.h/.cpp` | **v2 32bit Rangeエンコーダ** |
| `lib/tmg1-codec/include/tmg1/prediction.h/.cpp` | 予測フィルタ (None/Left/Up) |
| `lib/tmg1-codec/include/tmg1/decoder.h/.cpp` | tmg1::Decoder クラス (Tmg1Stream I/F) |
| `lib/tmg1-codec/include/tmg1/encoder.h/.cpp` | tmg1::Encoder クラス (Tmg1Stream I/F) |
| `lib/tmg1-codec/include/tmg1/arduino_stream.h` | Arduino Stream → Tmg1Stream アダプタ |
| `lib/tmg1-codec/c_api/tmg1_c.h/.cpp` | extern "C" API (Rust FFI用) |
| `lib/tmg1-codec/test/test_rice.cpp` | Riceコーダ ユニットテスト |
| `lib/tmg1-codec/test/test_range.cpp` | Rangeコーダ ユニットテスト |
| `lib/tmg1-codec/test/test_prediction.cpp` | 予測フィルタ ユニットテスト |
| `lib/tmg1-codec/test/test_codec.cpp` | エンコード→デコード ラウンドトリップテスト |
| `lib/tmg1-codec/CMakeLists.txt` | デスクトップビルド・テスト設定 |
| `lib/tmg1-codec/library.properties` | Arduino ライブラリ認識用 |
| `src/main.cpp` | 新API (tmg1::Decoder + tmg1_stream_from_arduino) に更新 |
| `test/test_Tmg1Decoder.cpp` | 新APIに対応したテストに書き換え |
| `test/test_main.cpp` | 更新 |
| `platformio.ini` | native 環境を新API対応に更新 |

#### 決定事項

| 決定 | 内容 | 理由 |
|---|---|---|
| 配置場所 | `lib/tmg1-codec/` (tmg1-arduino内) | 今すぐビルドが動く。後でサブモジュール化 |
| `_low` の型 | uint32 (デコーダ), uint32 (エンコーダ) | 32bit wrap-aroundで動作 |
| フラッシュバイト数 | 4バイト (v1は8バイト) | v2 32bit化に対応 |

#### Rangeコーダ v2 実装詳細（重要）
- デコーダ: `_low`, `_range`, `_code` すべて uint32
- `scaledCode = ((code - low + 1) * total - 1) / range` → uint64で計算（積 ≦ 2^52）
- 正規化条件: `(_low ^ (_low + _range)) < (1u << 24) || _range < (1u << 23)`
- 初期化: 4バイト読み込み `_code = (b3<<24)|(b2<<16)|(b1<<8)|b0`
- エンコーダflush: `_low >> 24` を4回出力

#### 次回セッションで取り組む内容
1. Phase 2: `lib/tmg1-codec` を独立リポジトリ化してサブモジュール化
2. Phase 3: `tmg1-rust-cli` 新規作成

---

### 2026-06-07 セッション 3

#### 作業内容
旧ファイル削除・テスト完全化・GitLab CI改善。

#### 完了したこと

| 作業 | 詳細 |
|---|---|
| 旧ファイル削除 | `src/` から6ファイル削除 (FrequencyModel.h, RangeDecoder.h, RiceBitReader.h/.cpp, Tmg1Decoder.h/.cpp) |
| テスト完全化 | `test_decode_file_from_simple_tmg1` を実装。tmpfile経由ラウンドトリップテスト。11 PASSED / 0 SKIPPED |
| io.h 拡張 | stdio (FILE*) ベース Tmg1Stream ヘルパー追加 (`tmg1_file_read/write/tell/seek`, `tmg1_stream_from_file_read/write`) |
| CI改善 | `.gitlab-ci.yml` にキャッシュ・rules・CMakeビルドジョブ追加 |

#### 決定事項

| 決定 | 内容 | 理由 |
|---|---|---|
| ファイルテストの実装 | tmpfile()でエンコード→デコードラウンドトリップ | パス依存なし・自己完結・FILE* I/Oを実際にテスト |
| stdio ヘルパー配置 | io.h 内に `#if !defined(ARDUINO)` ガードで追加 | Arduino環境では不使用、デスクトップテストで有用 |

#### 次回セッションで取り組む内容
1. Phase 2: `lib/tmg1-codec` を独立リポジトリ化してサブモジュール化
2. Phase 3: `tmg1-rust-cli` 新規作成
