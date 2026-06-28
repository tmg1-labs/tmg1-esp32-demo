# TMG1コーデック

@.claude/architecture.md
@.claude/coding-style.md
@.claude/workflows.md
@.claude/context/current-sprint.md
@.claude/context/known-issues.md

## Quick facts
- 言語: C++17 (`-std=gnu++17`)
- 対象: Arduino / ESP32 (esp32dev, seeed_xiao-esp32c3, super-mini-k2) + native(デスクトップ)
- ビルド: PlatformIO / コーデック単体は CMake
- テスト: Unity (PlatformIO `pio test -e native` / CMake `ctest`)
- CI: GitLab CI (`.gitlab-ci.yml`)
- フォーマッタ: clang-format (Google ベース, ColumnLimit 120)
- フォーマット仕様: TMG1 (v2 / 32bit Range コーダ)
- コーデック本体: `lib/tmg1-codec`（サブモジュール、別リポジトリ。CLI/Arduino で共有）

## 関連リポジトリ
- `tmg1-codec`: 共通C++コーデックライブラリ（このリポジトリにサブモジュールとして同梱）
- `tmg1-cli`: Rust 製 CLI（codec を FFI で利用、dotnet版とパリティ進行中）

## Claudeへの指示
- 方針の決定や修正に関する意図や経緯があれば記録していくこと。
- セッションの記録は `session-record` スキル、初期化は `code-project-init` スキルを使う。
- 長期の決定経緯・セッション履歴は `.claude/context/session-history.md`、過去のエラー解決は
  `.claude/context/errors-log.md` を参照。
