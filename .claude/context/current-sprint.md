# 現在の作業コンテキスト

最終更新: 2026-06-30（tmg1-arduino→tmg1-esp32-demo 改名・arduino README/LICENSE 整備・クロスリンク集約）

## 今やっていること
- **tmg1-arduino → `tmg1-esp32-demo` へ改名 + README(英/日)・LICENSE 整備**（2026-06-30、commit `390a759`、main、未 push）。
  - 位置づけを明確化: 取り込む Arduino ライブラリは **`tmg1-codec` 単体で完結**（`library.properties` + `arduino_stream.h` を同梱）。本リポジトリは「codec を実機で動かす ESP32 リファレンス（サンプル）」に過ぎない。
  - リポジトリ名を `tmg1-arduino`→`tmg1-esp32-demo` に改名決定（旧名はプレイヤー主体に見えるため。ESP32 専用デモという実態へ寄せる）。
  - GitLab テンプレートのボイラープレート README を破棄し日英 README を新規作成 + `LICENSE`（MIT、codec/cli と同一）を追加。
  - **クロスリンク方針を決定**: README 間の兄弟カタログ記述は保守コスト（リポジトリ追加・改名で全 README を修正する羽目になる）。一覧の正本は組織トップ `github.com/tmg1-labs`（`.github/profile/README.md`）に集約し、各 README には機能リンク（依存先 codec / 仕様書 / 入力生成 cli）＋「Part of TMG1 Labs」固定ポインタのみ残す。
  - 改名と方針を全リポジトリの**公開リンク**へ反映しコミット（内部の履歴メモ `.claude/context/*`・`docs/implementation-plan.md`・`architecture.md` は当時の事実として据え置き）:
    - `tmg1-codec` commit `0855972`（兄弟列挙削除＋組織ポインタ＋**LICENSE をコミット**＝懸案解消）。
    - `tmg1-cli` commit `73e52f3`（兄弟列挙削除＋組織ポインタ）。
    - `gitlab-profile` commit `c096b4f`（目次・仕様書の改名追随）。
  - 4 リポジトリとも **未 push**。
- 一連リポジトリの **GitHub 公開準備**（2026-06-29〜）。
  - GitHub 組織 `tmg1-labs` を作成済み（個人で Free 組織を作成）。`gitlab` の `tsumugi` は取得済みだったため `tmg1-labs` を採用。
  - 組織の `.github` リポジトリ（= `gitlab-profile` リポジトリ）を **再構成・コミット済み**（2026-06-30、commit `3002954`、未 push）。
    - `profile/README.md`: 組織ランディング + 各リポジトリ目次（codec/arduino/cli）を新規作成。
    - `docs/tmg1-format.md`（英）/ `docs/tmg1-format.ja.md`（日）: 仕様書を分離。日本語版を新規作成、英版は旧 README を整形しテーブル崩れを修復。
    - `SECURITY.md`（英/日、連絡先 `faces@rainorshine.asia`）を新規作成。
    - ルート `README.md` をリポジトリ説明（中身の目次）へ差し替え（旧 README の仕様書本文は docs へ移設）。
    - リンクは `github.com/tmg1-labs/<repo>` 前提で統一。
  - `tmg1-codec` の README を GitHub へ同期・**コミット済み**（2026-06-30、commit `1296366`、未 push）。
    - 兄弟リポジトリリンクを `gitlab.com/seizu/tsumugi` → `github.com/tmg1-labs` へ。
    - 仕様書ポインタの「公開予定」を `tmg1-labs/.github` の docs リンクへ差し替え（英/日）。
    - 注: README.md/README.ja.md は今回が初コミット（従来 untracked だった）。**LICENSE はまだ untracked のまま残置**。
  - `tmg1-cli` の **README(英/日)+LICENSE を新規作成・コミット済み**（2026-06-30、commit `c7f3685`、main、未 push）。
    - codec と同じ日英 README 構成（`README.md` 英・主 / `README.ja.md` 日、冒頭で相互リンク）。
    - encode/transcode/decode/info の全フラグ・既定値を `src/main.rs` の clap 定義に一致させて記載。
    - 兄弟リポジトリ/仕様書リンクは `github.com/tmg1-labs` 規約に統一（codec と同じ）。
    - `LICENSE` は codec と完全同一（MIT、`Copyright (c) 2026 surface0`）。
    - `cargo build --release` 成功 + 全サブコマンド `--help` と README 記述の一致を実機照合済み。
- `tmg1-codec` の **README 整備**（2026-06-29、ファイルは codec 本流リポジトリ側）。
  - `README.md`（英・主）/ `README.ja.md`（日）を新規作成。冒頭で相互リンク。
  - `LICENSE`（MIT、`Copyright (c) 2026 surface0`）を新規作成。
  - 説明文中の「v2」表記を削除（v1 は未公開のため公開ドキュメントで v2 を語らない）。FileHeader の `version=2` 等のバイナリ事実は残置。
  - 「TMG1 Format」節はバイト単位の表（FileHeader/FrameHeader/TMGX/Range コーダ不変条件）を削除し、高レベル概要 + 「正本は独立仕様書（公開予定）」ポインタに縮小（仕様書との重複回避）。
- 実機 OLED 向けの動画素材の調整（ffmpeg パイプライン）。表示極性は XOR 反転で対処（2026-06-28 burenai 実機 OK）。
- `tmg1-cli` を dotnet 版 CLI と機能パリティにする作業はほぼ完了
  （prediction / rice-mode・rice-k / scd / vfr / index / info+transcode を実装済み）。

## 直近の未確定・未完了
- `tmg1-codec` の README/LICENSE は **コミット済み**（最新 commit `0855972`。LICENSE も同コミットで追加完了＝懸案解消）。
- 全リポジトリのコミット（`gitlab-profile` `c096b4f` / `tmg1-codec` `0855972` / `tmg1-cli` `73e52f3` / `tmg1-esp32-demo`(旧 arduino) `390a759`）はいずれも **未 push**。
- 仕様書リンクは `github.com/tmg1-labs/.github/blob/main/docs/tmg1-format(.ja).md` を指す前提。**`.github` リポジトリを GitHub へ公開・push するまでリンク切れ**（公開順序: 先に .github 側を出す）。
- GitHub 公開方式（個人で公開→組織へ transfer / 最初から組織直下）は未確定。`tmg1-labs/` 直下でない形になる場合は全 README のリンク再調整が必要。
- README の CI 説明は「GitLab（`.gitlab-ci.yml`）」のまま残置（事実）。GitHub Actions 移行時に要更新。
- `transcode` は ffmpeg 未導入のため e2e 未検証（実装は完了）。
- `info` 詳細化 + `transcode` のコミットは未実施の可能性あり（push 状況は要確認）。

## 一時的な制約・注意事項
- `data/` 配下のサンプル `.tmg1` は再エンコードで差し替えることがある（git status 参照）。
- **訂正**: standalone の `d:/workspace/TsuMuGi/tmg1-codec` 別クローンは codec の **正本作業コピー**（README/LICENSE 整備はここで実施、HEAD `0855972`）。従来の「古いコミット・放置可」記述は誤り。arduino 側の `lib/tmg1-codec` はサブモジュールで別管理。codec を push 後にサブモジュールポインタを同期する。

## 次にやること
- 全リポジトリのコミットを GitHub `tmg1-labs` へ push。**先に `.github`（gitlab-profile）を公開**して仕様書リンクのリンク切れを解消する。
- codec を push 後、`tmg1-esp32-demo` の `lib/tmg1-codec` サブモジュールポインタを最新 codec（`0855972`）へ同期。
- GitHub 上でリポジトリ名 **`tmg1-esp32-demo`** で作成/公開（ローカルフォルダ名 `tmg1-arduino`・git remote は未変更のまま。実フォルダを改名する場合は `.claude/architecture.md` の構成図と本ファイルの旧名記述も追従更新する）。
- README の CI 説明は GitLab のまま残置（事実。GitHub Actions 移行時に要更新）。
- 実機での表示確認とサンプル動画の最終調整。

## 参考
- 決定経緯・セッション履歴の詳細は `session-history.md`、過去のエラー解決は `known-issues.md` / `errors-log.md`。
