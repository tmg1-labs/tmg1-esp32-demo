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
