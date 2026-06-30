# 既知の問題・注意事項

> 過去のエラー解決の原文は `errors-log.md`。ここはハマりやすい地雷の要点を集約する。

## ビルド・CI

### CMake + Unity: link language / setUp 未定義
- **症状**: `Cannot determine link language for target "unity"` / `undefined reference to 'setUp'` 等。
- **原因**: `project()` に C 言語未宣言 / 生 Unity はテストに `setUp()`/`tearDown()` が必要（PlatformIO は自動生成）。
- **回避策**: `project(tmg1-codec C CXX)` とし、`test/test_common.c` に空スタブを定義して各テストへリンク。

### GitLab CI のサブモジュール clone 失敗
- **症状**: SSH URL だと鍵なしで失敗、相対 URL でも 403。
- **回避策**: `.gitmodules` を相対 URL（`../tmg1-codec.git`）にし、サブモジュール側プロジェクトの
  Settings → CI/CD → Job token permissions に本リポジトリを許可リスト追加。

### GitHub Actions: 他ホスト submodule の recursive 取得失敗（2026-07-01 移行時）
- **症状/懸念**: arduino の `.gitmodules` は `lib/tmg1-codec`(相対 URL) と `docs/specification`(gitlab.com 絶対 URL) の2つ。
  `actions/checkout` の `submodules: recursive` で取得すると、ビルドに不要な docs/specification まで gitlab.com から
  fetch しに行き、非公開/到達不可だとジョブが失敗しうる。
- **回避策**: recursive を使わず、ビルドに必要な submodule だけ明示 init する
  （`git submodule update --init lib/tmg1-codec`）。相対 URL `../tmg1-codec.git` は superproject の origin
  （GitHub）に対して解決されるため `tmg1-labs/tmg1-codec.git` を指す。
- **副次の前提**: 相対 URL 解決のため **codec を先に GitHub 公開**してから cli/arduino を push する（公開順序依存）。
- 注: cli は submodule が codec 1個のみなので `submodules: recursive` で問題なし。複数 submodule で一部が他ホストの
  リポジトリだけ要注意。

### GitHub Actions: ローカル未検証ジョブ（PlatformIO）
- **状況**: arduino の `test_native`(`pio test -e native`)は開発機に PlatformIO 未導入で実走未検証のまま移行。
  cmake 系ジョブは WSL gcc で代替検証済み（全19テスト PASS）。
- **対応**: GitHub push 後の初回ワークフロー実行で `test_native` の成否を必ず確認する。

### GitHub Actions: setup-python の `cache: pip` で依存ファイル不在エラー（2026-07-01 実証）
- **症状**: arduino の `test_native` 初回実行が `actions/setup-python@v5` ステップで失敗。
  `No file in ... matched to [**/requirements.txt or **/pyproject.toml]`。
- **原因**: `cache: pip` は `requirements.txt`/`pyproject.toml` のハッシュをキャッシュキーにするが、
  本リポジトリにはどちらも無い（PlatformIO は `pip install platformio` で都度導入）ため、キー生成に失敗する。
- **回避策**: `setup-python` から `cache: pip` を外す（commit `cb6bfbe`）。PlatformIO パッケージ本体の
  キャッシュは `actions/cache`(`~/.platformio`)で別途行っているので影響なし。再実行で test_native/test_cmake とも success。
- **教訓**: `setup-python` の pip キャッシュは「pip の依存ファイルが存在するプロジェクト」専用。
  ツールをアドホックに `pip install` するだけの構成では使わない。

## コーデック実装

### Rice 大きい K で出力が黙って切り詰められる（重要）
- **症状**: `--rice-mode fixed --rice-k 7` で encode→decode が不一致（出力が空）。
- **原因**: 圧縮先バッファ不足。Rice(k=7) は最悪約 9 倍に膨張するが、`tmg1_mem_write` は溢れを黙って切り詰め、
  `RiceBitWriter` は `writeByte` の失敗を無視する。
- **回避策**: 圧縮先バッファを `_frameBufferSize * 10 + height + 64` に拡大。
- **将来改善**: writeBit 系が writeByte のエラーを伝播していないのが根本的な弱点。

### Rice エンコーダのランレングス遷移バグ
- **症状**: 行データがずれてデコード不能。
- **原因**: 遷移時に `runLen=0` リセットし、遷移点ピクセルが新ランにカウントされない。
- **回避策**: 遷移時 `runLen = (x < width) ? 1 : 0` として現在ピクセルを新ランに含める。

### FileHeader サイズ変更時のテストデータ不足
- **症状**: 不正シグネチャ/バージョンのテストが ReadError(-3) で早期失敗。
- **原因**: FileHeader は 16 バイト（signature4+version1+flags1+width2+height2+timebaseNum2+timebaseDen2+keyInterval2）。
- **回避策**: 構造変更時はテストデータのバイト数も必ず合わせる。

## ターゲット差異

### RISC-V (ESP32-C3) での `std::max` 型不一致
- **症状**: x86(native/esp32dev) では通るが C3 でテンプレート型推論失敗。
- **原因**: RISC-V では `uint32_t = unsigned long`、リテラル `1u` は `unsigned int` で型が異なる。
- **回避策**: `std::max((uint32_t)1u, ...)` と明示キャスト。

## 開発環境

### WSL を `bash -lc '...'` でインライン実行すると変数展開が消える
- **症状**: シングルクオート内の `$VAR`/`$(...)` が空になり、検証が全件 MISMATCH（コマンドは成立し気づきにくい）。
- **回避策**: 処理を `.sh` に書き出して `wsl bash -lc 'bash /mnt/d/.../script.sh'` で実行。
  `/mnt` パスは `-lc` 文字列の**中**に置く（引数に直接渡すと Git Bash がパス変換する）。

### git commit が `1Password: failed to fill whole buffer` で失敗
- **症状**: `tmg1-codec` で `git commit` が `1Password: failed to fill whole buffer` / `fatal: failed to write commit object` で落ちる（他リポジトリは成功）。
- **原因**: codec は `commit.gpgsign=true` + `gpg.format=ssh`（1Password の SSH 署名）。1Password がロック中だと非対話セッションで署名鍵を取得できない。
- **回避策**: 1Password をアンロックして**同じコミットを再実行**すれば通る（ステージは残っている）。`--no-gpg-sign` で握りつぶさないこと。

## 地雷・禁止事項
- v1 互換コードを持ち込まない（v2 のみ）。
- codec の分岐コピーを増やさない（本流を直し、サブモジュールポインタで同期）。
- lossy-bias はオミット済み（オープンループで残像蓄積。再開はクローズドループが正攻法だが本用途では非推奨）。
