# 現在の作業コンテキスト

最終更新: 2026-07-01（codec を submodule → PlatformIO lib_deps 移行 + Node20 警告対応。**全リポジトリ push 済み**）

## 今やっていること
- **codec を submodule から PlatformIO `lib_deps`(git タグ)へ移行**（2026-07-01、esp32-demo。**push 済み・CI green**）。
  - 背景: codec が GitHub 公開済みのため、サブモジュール同梱をやめ「純粋な消費者」に寄せた。バージョンは git タグで固定。
  - **codec repo**（正本 `d:/workspace/TsuMuGi/tmg1-codec`）: `library.properties` の url を gitlab→github に修正（commit `a4aa9a9`）→ **`v0.2.0` タグを打って push**。lib_deps はこのタグを参照。
  - **esp32-demo**（commit `147bc70`→`c8cbdd9`→`a77a1c6`→`b551938`、push 済み）:
    - `platformio.ini` 全 env(esp32dev/seeed/native)の `lib_deps` に `https://github.com/tmg1-labs/tmg1-codec.git#v0.2.0` を追加。native の `-I lib/tmg1-codec/include` は撤去（include/ は PlatformIO が自動付与）。
    - submodule `lib/tmg1-codec` 撤去（`.git/config`・`.git/modules` 残骸も掃除）。`docs/specification`(gitlab 参照・未 init 残骸)も `.gitmodules` ごと削除（→ `.gitmodules` 自体消滅）。
    - CI: submodule init 撤去 + 冗長な `test_cmake` ジョブ削除（codec 単体テストは codec 本体 CI が担う）→ `test_native` 1ジョブに。
    - ドキュメント(CLAUDE.md/architecture.md/workflows.md/README 英日)を lib_deps 運用へ更新。
  - **検証**: esp32-demo CI **green**。`test_native` の「Native tests」通過 = `pio test -e native` が lib_deps 経由で codec を取得・コンパイルして Unity 成功。加えて **lib_deps 移行後のファーム実機ビルド・動作を確認済み（2026-07-01）** ＝ 実機再生まで含めて移行完了。
  - **注**: `tmg1-cli` は submodule のまま（lib_deps は PlatformIO 専用。cli は `build.rs`+`cc` で codec を直接コンパイルするため対象外）。
- **submodule ポインタ同期のずれを是正**（移行前の中間作業、commit `147bc70`）。esp32-demo が指す codec が `90905a7` で 5 コミット遅れ（docs/CI/LICENSE のみ、コード差分なし）→ `8dfdfea` へ同期。この過程で submodule remote がまだ gitlab を指していたため `git submodule sync` で GitHub へ貼り直し。※直後の lib_deps 移行で submodule 自体を撤去。
- **CI の Node20 deprecation 警告に対応**（2026-07-01、esp32-demo commit `b551938`、push 済み・CI green・annotations 0）。
  - `actions/setup-python@v5`・`actions/cache@v4` が Node20 ランタイムで非推奨警告 → Node24 対応の **v6 系**(`setup-python@v6.3.0`/`cache@v6.1.0`)へ更新。checkout は既に v5(Node24)。

## 今やっていること（過去分）
- **CI を GitLab CI → GitHub Actions へ移行**（2026-07-01、codec/cli/arduino の3リポジトリ、各 main、いずれも本セッションでコミット）。
  - 各リポジトリで `.github/workflows/ci.yml` を新規追加し `.gitlab-ci.yml` を `git rm`（一本化）。
  - 共通方針: トリガは `push`(branches: main, feature/**) + `pull_request`。runner は `ubuntu-latest`（rust/g++/cmake/python 同梱のため image 指定・apt 不要）。
  - **codec**（正本作業コピー `d:/workspace/TsuMuGi/tmg1-codec`）: `test_cmake` 1ジョブ。Unity は `actions/checkout` で **v2.6.0 タグ固定**取得（旧 CI の master 追従 clone から変更、再現性向上）→ cmake build → ctest。
  - **cli**: `build_test` 1ジョブに統合（GitLab は build/test 別ジョブ。両者とも submodule+g++ 必要で test は build 成果物依存のため統合）。`actions/checkout`(submodules: recursive) → `Swatinem/rust-cache@v2` → `cargo build --release` → `cargo test` → release バイナリ `tmg1` を `actions/upload-artifact@v4`(7日)。`build.rs` が `cc` で codec C++ をコンパイルするため submodule 必須。
  - **arduino**: 2ジョブ（`test_native`=PlatformIO `pio test -e native`、`test_cmake`=cmake/ctest）。**docs/specification(gitlab.com 他ホスト submodule)は不要なので recursive にせず `git submodule update --init lib/tmg1-codec` で codec のみ init**（recursive だと gitlab 依存で失敗リスク）。GitLab 手動 cache は `setup-python` の pip cache + `actions/cache`(~/.platformio) に置換。
  - 相対 URL submodule `../tmg1-codec.git` は GitHub 上で `tmg1-labs/tmg1-codec.git` に解決される前提。よって **公開順序は codec → cli/arduino**。
  - 検証: codec/arduino の cmake ジョブ相当は WSL gcc で全19テスト PASS 確認。cli は `cargo clean` 後のクリーンビルド成功（codec を cc でコンパイル）+ `cargo test`(0件)。**arduino の `pio test -e native` はローカルに PlatformIO 未導入で実走未検証**（定義は旧 CI のコマンドを踏襲）。GitHub push 後の初回実行で最終確認。
  - 注: README の CI 説明は「GitLab（`.gitlab-ci.yml`）」のまま残置（GitHub Actions へ要更新）。arduino `.gitmodules` の docs/specification はまだ gitlab.com を指す（公開時に向け先要判断）。
- **tmg1-arduino → `tmg1-esp32-demo` へ改名 + README(英/日)・LICENSE 整備**（2026-06-30、commit `390a759`、main、未 push）。
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
- **更新(2026-07-01)**: 全リポジトリは **GitHub `tmg1-labs` へ push 済み**（下記「未 push」記述は解消済み）。codec は `v0.2.0` タグ付き。esp32-demo の submodule は撤去され lib_deps 運用に移行済み。
- `tmg1-codec` の README/LICENSE は **コミット済み**（LICENSE も追加完了＝懸案解消。現 HEAD は `a4aa9a9`＝library.properties url 修正 + tag `v0.2.0`）。
- ~~全リポジトリのコミットはいずれも未 push~~ → **push 済みに更新**（2026-07-01）。
- 仕様書リンクは `github.com/tmg1-labs/.github/blob/main/docs/tmg1-format(.ja).md` を指す前提。**`.github` リポジトリを GitHub へ公開・push するまでリンク切れ**（公開順序: 先に .github 側を出す）。
- GitHub 公開方式（個人で公開→組織へ transfer / 最初から組織直下）は未確定。`tmg1-labs/` 直下でない形になる場合は全 README のリンク再調整が必要。
- CI は GitHub Actions へ移行済み（2026-07-01）。各 README（codec/cli/arduino の英・日 計6ファイル）の CI 説明も GitHub Actions 記述へ**更新済み**（2026-07-01）。
- `transcode` は ffmpeg 未導入のため e2e 未検証（実装は完了）。
- `info` 詳細化 + `transcode` のコミットは未実施の可能性あり（push 状況は要確認）。

## 一時的な制約・注意事項
- `data/` 配下のサンプル `.tmg1` は再エンコードで差し替えることがある（git status 参照）。
- **訂正**: standalone の `d:/workspace/TsuMuGi/tmg1-codec` 別クローンは codec の **正本作業コピー**（現 HEAD `a4aa9a9` + tag `v0.2.0`）。従来の「古いコミット・放置可」記述は誤り。~~arduino 側の lib/tmg1-codec はサブモジュール~~ → **esp32-demo は submodule を撤去し lib_deps(git タグ `#v0.2.0`)で消費**。codec 修正時はタグを切って esp32-demo の `platformio.ini` の `#vX.Y.Z` を上げて同期する。
- codec を上げるフローは submodule ポインタ更新 →「codec でタグ push → esp32-demo の lib_deps バージョン更新」の2手順に変わった。

## 次にやること
- **arduino/esp32-demo の `test_native`(PlatformIO)は GitHub Actions で実走・green 確認済み**（lib_deps 経由 codec 取得含む。ローカル PlatformIO 未導入の懸案は CI で解消）。
- ~~docs/specification(gitlab.com) の向け先判断~~ → **submodule ごと削除で解消**（仕様書は README の `tmg1-labs/.github` リンクで参照）。
- ~~全リポジトリを push~~ → **完了**。
- ~~codec push 後の submodule ポインタ同期~~ → **lib_deps 移行で不要化**。
- **ローカルフォルダ改名（手動・未実施のまま）**: `tmg1-arduino` → `tmg1-esp32-demo`、`gitlab-profile` → `tmg1-labs.github`（リポジトリ実体は `tmg1-labs/.github`）。GitHub 公開・remote 切替・README 整備は実施済みで、残るのはローカルフォルダ名のみ。セッションがフォルダのハンドルを掴むため改名はエディタ/セッションを閉じてから実施する。git remote / lib_deps はフォルダ名に依存しないため再設定不要。
- ~~他リポジトリ(codec/cli)の Node20 警告確認~~ → **完了(2026-07-01)**。codec は `checkout@v5` のみで該当なし(annotations 0)。cli は `upload-artifact@v4`(node20)→**v7(node24)**へ更新(commit `dc77383`、push 済み・CI green・annotations 0)。`rust-cache@v2`(=v2.9.1)・`checkout@v5` は既に node24。
- ~~ローカルフォルダ改名(手動・未実施)~~ → **完了**（`tmg1-arduino`→`tmg1-esp32-demo`、`gitlab-profile`→`tmg1-labs.github`)。
- ~~実機での表示確認~~ → **lib_deps 移行後の実機ビルド・動作 確認済み(2026-07-01)**。残るはサンプル動画の最終調整のみ。

## 参考
- 決定経緯・セッション履歴の詳細は `session-history.md`、過去のエラー解決は `known-issues.md` / `errors-log.md`。
