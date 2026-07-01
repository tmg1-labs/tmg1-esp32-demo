# 現在の作業コンテキスト

最終更新: 2026-07-01（submodule→lib_deps移行 + CI Node20対応、いずれも完了・push済み）

## 今やっていること
- **codec を submodule から PlatformIO `lib_deps`(git タグ)へ移行**（2026-07-01、**push 済み・CI green**）。
  - 背景: `tmg1-codec` が GitHub 公開済みのため、サブモジュール同梱をやめ「純粋な消費者」に寄せた。
    バージョンは git タグで固定（codec本体側の対応は `tmg1-codec` の current-sprint.md 参照）。
  - 本リポジトリ側（commit `147bc70`→`c8cbdd9`→`a77a1c6`→`b551938`、push 済み）:
    - `platformio.ini` 全 env(esp32dev/seeed/native)の `lib_deps` に
      `https://github.com/tmg1-labs/tmg1-codec.git#v0.2.0` を追加。native の
      `-I lib/tmg1-codec/include` は撤去（include/ は PlatformIO が自動付与）。
    - submodule `lib/tmg1-codec` 撤去（`.git/config`・`.git/modules` 残骸も掃除）。
      `docs/specification`(gitlab 参照・未 init 残骸)も `.gitmodules` ごと削除
      （→ `.gitmodules` 自体消滅）。
    - CI: submodule init 撤去 + 冗長な `test_cmake` ジョブ削除（codec 単体テストは codec 本体 CI
      が担う）→ `test_native` 1ジョブに。
    - ドキュメント(CLAUDE.md/architecture.md/workflows.md/README 英日)を lib_deps 運用へ更新。
  - **検証**: CI **green**。`test_native` の「Native tests」通過 = `pio test -e native` が
    lib_deps 経由で codec を取得・コンパイルして Unity 成功。加えて **lib_deps 移行後の
    ファーム実機ビルド・動作を確認済み（2026-07-01）** ＝ 実機再生まで含めて移行完了。
  - **注**: `tmg1-cli` は今も submodule のまま（lib_deps は PlatformIO 専用。cli は `build.rs`+`cc`
    で codec を直接コンパイルするため対象外。詳細は `tmg1-cli` 側参照）。
- **CI の Node20 deprecation 警告に対応**（2026-07-01、commit `b551938`、push 済み・CI green・
  annotations 0）。
  - `actions/setup-python@v5`・`actions/cache@v4` が Node20 ランタイムで非推奨警告 →
    Node24 対応の **v6 系**(`setup-python@v6.3.0`/`cache@v6.1.0`)へ更新。checkout は既に v5(Node24)。

## 今やっていること（過去分）
- **submodule ポインタ同期のずれを是正**（移行前の中間作業、commit `147bc70`）。本リポジトリが
  指す codec が `90905a7` で 5 コミット遅れ（docs/CI/LICENSEのみ、コード差分なし）→ `8dfdfea` へ
  同期。この過程で submodule remote がまだ gitlab を指していたため `git submodule sync` で
  GitHub へ貼り直し。※直後の lib_deps 移行で submodule 自体を撤去。
- **CI を GitLab CI → GitHub Actions へ移行**（2026-07-01、本リポジトリ側対応分。他リポジトリの
  対応は各リポジトリの current-sprint.md 参照）。
  - `.github/workflows/ci.yml` を新規追加し `.gitlab-ci.yml` を `git rm`。
  - 2ジョブ（`test_native`=PlatformIO `pio test -e native`、`test_cmake`=cmake/ctest。後に
    `test_cmake` は codec本体CIとの重複のため削除し `test_native` 1ジョブへ整理）。
  - **docs/specification(gitlab.com 他ホスト submodule)は不要なので recursive にせず
    `git submodule update --init lib/tmg1-codec` で codec のみ init**（recursive だと gitlab
    依存で失敗リスク）。GitLab 手動 cache は `setup-python` の pip cache + `actions/cache`
    (~/.platformio) に置換。
  - 相対 URL submodule `../tmg1-codec.git` は GitHub 上で `tmg1-labs/tmg1-codec.git` に解決
    される前提（公開順序制約の詳細は `tmg1-labs.github` 参照）。
- **`tmg1-arduino` → `tmg1-esp32-demo` へ改名 + README(英/日)・LICENSE 整備**（2026-06-30、
  commit `390a759`）。
  - 位置づけを明確化: 取り込む Arduino ライブラリは `tmg1-codec` 単体で完結
    （`library.properties` + `arduino_stream.h` を同梱）。本リポジトリは「codec を実機で動かす
    ESP32 リファレンス（サンプル）」に過ぎない。
  - リポジトリ名を `tmg1-arduino`→`tmg1-esp32-demo` に改名決定（旧名はプレイヤー主体に見えるため。
    ESP32 専用デモという実態へ寄せる）。
  - GitLab テンプレートのボイラープレート README を破棄し日英 README を新規作成 +
    `LICENSE`（MIT、codec/cli と同一）を追加。
  - クロスリンク方針（兄弟リポジトリ一覧は組織トップに集約）の決定経緯は
    `tmg1-labs.github` の session-history.md 参照。
- 実機 OLED 向けの動画素材の調整（ffmpeg パイプライン）。表示極性は XOR 反転で対処
  （2026-06-28 burenai 実機 OK）。

## 一時的な制約・注意事項
- `data/` 配下のサンプル `.tmg1` は再エンコードで差し替えることがある（git status 参照）。
- codec を上げるフローは「`tmg1-codec` でタグ push → 本リポジトリの `platformio.ini` の
  `lib_deps` の `#vX.Y.Z` を上げる」の2手順（旧submoduleポインタ更新方式から変更済み）。

## 次にやること
- `test_native`(PlatformIO)は GitHub Actions で実走・green 確認済み（lib_deps 経由 codec
  取得含む）。実機での表示確認も完了。残るはサンプル動画の最終調整のみ。

## 参考
- 決定経緯・セッション履歴の詳細は `session-history.md`、過去のエラー解決は
  `known-issues.md` / `errors-log.md`。
- codec本体・cli・組織プロフィールの作業状況は、それぞれ `tmg1-codec` /
  `tmg1-cli` / `tmg1-labs.github` の `.claude/context/current-sprint.md` を参照。
