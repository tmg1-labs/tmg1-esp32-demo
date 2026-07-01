# TMG1コーデック

@.claude/architecture.md
@.claude/coding-style.md
@.claude/workflows.md
@.claude/context/current-sprint.md
@.claude/context/known-issues.md

## Quick facts
- 言語: C++17 (`-std=gnu++17`)
- 対象: Arduino / ESP32 (esp32dev, seeed_xiao-esp32c3, super-mini-k2) + native(デスクトップ)
- ビルド: PlatformIO
- テスト: Unity (PlatformIO `pio test -e native`)
- CI: GitHub Actions (`.github/workflows/ci.yml`)
- フォーマッタ: clang-format (Google ベース, ColumnLimit 120)
- フォーマット仕様: TMG1 (v2 / 64bit Range コーダ。詳細は `tmg1-codec` 参照)
- コーデック本体: `tmg1-codec`（別リポジトリ。PlatformIO `lib_deps` の git タグ `#v0.2.0` で取得。CLI/Arduino で共有）

## 関連リポジトリ
- `tmg1-codec`: 共通C++コーデックライブラリ（PlatformIO `lib_deps` の git タグで取得）
- `tmg1-cli`: Rust 製 CLI（codec を FFI で利用、dotnet版とほぼパリティ完了）

## Claudeへの指示
- 方針の決定や修正に関する意図や経緯があれば記録していくこと。
- **コーデックのアルゴリズム詳細（Range コーダ v2 の実装等）は本リポジトリでは扱わない**。
  `tmg1-codec` の `.claude/architecture.md` を参照すること。
- セッションの記録は `session-record` スキル、初期化は `code-project-init` スキルを使う。
- 長期の決定経緯・セッション履歴は `.claude/context/session-history.md`、過去のエラー解決は
  `.claude/context/errors-log.md` を参照。
