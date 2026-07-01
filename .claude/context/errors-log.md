# エラーログ（過去のハマりと解決の原文）

> 2 回以上試行した問題と解決法の詳細記録。要点だけ拾うなら `known-issues.md` を参照。
> codec実装のエラー（CMake+Unity/Rice/Range/RISC-V）は `tmg1-codec` の errors-log.md、
> cli固有のエラーは `tmg1-cli` の errors-log.md を参照。

## 1. GitLab CI サブモジュール SSH クローンエラー（歴史的経緯・解決済み/obsolete）

> **注**: この問題は当時の tmg1-arduino（本リポジトリの旧名）が GitLab CI + submodule
> 構成だった頃のもの。現在は GitHub Actions へ移行し、submodule自体も撤去（lib_deps運用）
> しているため無関係化している。履歴として残す。

**うまくいかなかったこと:**
`.gitmodules` に SSH URL (`git@gitlab.com:...`) を使うと、
CI ランナーに SSH 鍵がないためクローン失敗。

相対 URL (`../tmg1-codec.git`) に変更しても、
tmg1-codec 側で CI ジョブトークンのアクセス許可がなく 403 エラー。

**うまくいったこと:**
1. `.gitmodules` を相対 URL に変更
2. `tmg1-codec` の Settings → CI/CD → Job token permissions で
   `seizu/tsumugi/tmg1-arduino` を許可リストに追加

**次回のために:**
同一 GitLab インスタンスのサブモジュールは相対 URL を使い、
サブモジュール側プロジェクトのジョブトークン許可設定も忘れずに行うこと。

---

## 2. WSL を `bash -lc '...'` でインライン実行すると `$VAR`/`$(...)` が消える

**うまくいかなかったこと:**
PowerShell→Bashツール→`wsl bash -lc '...'` の多重ネストで、シングルクオート内の
`for M in ...; do ... $M ...` や `D=$(mktemp -d)` の変数展開が空になり、
ラウンドトリップが全件MISMATCHになる（コマンド自体は成立するため気づきにくい）。

**うまくいったこと:**
テスト内容を `.sh` ファイルに書き出し、`wsl bash -lc 'bash /mnt/d/.../script.sh'` で実行。
スクリプト内なら変数・コマンド置換が正常に効く。
注意: `/mnt/...` を `bash` の引数に直接渡すと Git Bash がパス変換するため、
`-lc` 文字列の**中**に `/mnt` パスを置くこと。

**次回のために:**
WSL越しに変数・ループ・コマンド置換を含む処理を流すときは、インラインでなく
スクリプトファイル化する。検証ハーネス(roundtrip.sh等、`tmg1-cli`側)が既にこの形なのもそのため。
（本問題は `tmg1-codec`/`tmg1-cli` 側の errors-log.md にも同内容を記録している）
