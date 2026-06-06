# MEMORY.md

## セッション履歴

---

### 2026-06-07 セッション

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

#### Rangeコーダ v2 定数（重要）
- `_range`: uint32、初期値 `0xFFFFFFFF`
- `_code`: uint32（初期読み込み4バイト）
- `_low`: uint64
- `BottomValue = 1 << 23`
- `TopValue = 1 << 24`
- 中間積最大 2^52 → uint64で完結、uint128不要

#### 次回セッションで取り組む内容
- `tmg1-codec` リポジトリ新規作成
- Phase 1 実装開始（types.h / io.h 設計から）
- 実装順序は `docs/implementation-plan.md` の Phase 1 に従う
