# 既知の問題・注意事項

> 過去のエラー解決の原文は `errors-log.md`。ここはハマりやすい地雷の要点を集約する。
> codec実装（Rice/Range/RISC-V）・cli固有（cargo publish等）・組織運用の問題は
> それぞれ `tmg1-codec` / `tmg1-cli` / `tmg1-labs.github` の known-issues.md を参照。

## ビルド・CI

### GitHub Actions: Node20 deprecation 警告（2026-07-01 対応）
- **症状**: ワークフロー実行の annotation に「Node.js 20 is deprecated ... forced to run on Node.js 24」。
  失敗ではないが警告が付く（`actions/cache@v4`・`actions/setup-python@v5` が該当）。
- **回避策**: Node24 対応の**メジャー v6 系**へ更新（`setup-python@v6`・`cache@v6`）。`checkout` は v5 で Node24 済み。
  更新後は annotations 0 で green。最新版は `gh api repos/<owner>/<repo>/releases/latest --jq .tag_name` で確認できる。
- **落とし穴（アクションごとに Node24 化のメジャーが違う）**: 「最新メジャー = Node24」とは限らない。
  `runs.using` を実確認すること（`gh api repos/<o>/<r>/contents/action.yml?ref=<tag> --jq .content | base64 -d | grep using`）。

### GitHub Actions: 他ホスト submodule の recursive 取得失敗（歴史的経緯・解消済み）
> **解決済み(2026-07-01)**: 本リポジトリは submodule を全撤去し codec を lib_deps(git タグ)で取得、
> `docs/specification`(gitlab)も削除したため本問題は消滅。以下は当時の記録として残す。

- **症状/懸念**: 当時の `.gitmodules` は `lib/tmg1-codec`(相対 URL) と `docs/specification`(gitlab.com
  絶対 URL) の2つ。`actions/checkout` の `submodules: recursive` で取得すると、ビルドに不要な
  docs/specification まで gitlab.com から fetch しに行き、非公開/到達不可だとジョブが失敗しうる。
- **回避策（当時）**: recursive を使わず、ビルドに必要な submodule だけ明示 init する
  （`git submodule update --init lib/tmg1-codec`）。相対 URL `../tmg1-codec.git` は superproject
  の origin（GitHub）に対して解決されるため `tmg1-labs/tmg1-codec.git` を指す。

### GitHub Actions: setup-python の `cache: pip` で依存ファイル不在エラー（2026-07-01 実証）
- **症状**: `test_native` 初回実行が `actions/setup-python@v5` ステップで失敗。
  `No file in ... matched to [**/requirements.txt or **/pyproject.toml]`。
- **原因**: `cache: pip` は `requirements.txt`/`pyproject.toml` のハッシュをキャッシュキーにするが、
  本リポジトリにはどちらも無い（PlatformIO は `pip install platformio` で都度導入）ため、キー生成に失敗する。
- **回避策**: `setup-python` から `cache: pip` を外す（commit `cb6bfbe`）。PlatformIO パッケージ本体の
  キャッシュは `actions/cache`(`~/.platformio`)で別途行っているので影響なし。再実行で test_native/test_cmake とも success。
- **教訓**: `setup-python` の pip キャッシュは「pip の依存ファイルが存在するプロジェクト」専用。
  ツールをアドホックに `pip install` するだけの構成では使わない。

## 開発環境

### WSL を `bash -lc '...'` でインライン実行すると変数展開が消える
- **症状**: シングルクオート内の `$VAR`/`$(...)` が空になり、検証が全件 MISMATCH（コマンドは成立し気づきにくい）。
- **回避策**: 処理を `.sh` に書き出して `wsl bash -lc 'bash /mnt/d/.../script.sh'` で実行。
  `/mnt` パスは `-lc` 文字列の**中**に置く（引数に直接渡すと Git Bash がパス変換する）。
- 同様の問題は `tmg1-codec`/`tmg1-cli` 側でも発見されている（各リポジトリのknown-issues.mdにも記録）。

## 地雷・禁止事項
- v1 互換コードを持ち込まない（v2 のみ、詳細はcodec側）。
- codec の分岐コピーを増やさない（本流は `tmg1-codec`。修正はそちらへ入れ、タグを上げて
  `lib_deps` のバージョン参照を更新する）。
