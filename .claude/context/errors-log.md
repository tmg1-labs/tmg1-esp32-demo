# エラーログ（過去のハマりと解決の原文）

> 2 回以上試行した問題と解決法の詳細記録。要点だけ拾うなら `known-issues.md` を参照。

## CMake + Unity テスト ビルドエラー集

### 1. `Cannot determine link language for target "unity"`

**うまくいかなかったこと:**
`project(tmg1-codec CXX)` と C++ のみ宣言した状態で Unity (.c ファイル) をビルドしようとした。

**うまくいったこと:**
`project(tmg1-codec C CXX)` に変更して C 言語も有効化。

**次回のために:**
CMakeLists.txt に C ライブラリ（Unity 等）を追加するときは、`project()` に `C` を含めること。

---

### 2. `undefined reference to 'setUp'` / `undefined reference to 'tearDown'`

**うまくいかなかったこと:**
PlatformIO (Unity) はスタブを自動生成するが、生の Unity ではテストファイルに `setUp()`/`tearDown()` が必要。
テストファイルに定義がなかったためリンクエラー。

**うまくいったこと:**
`test/test_common.c` に空の `setUp()`/`tearDown()` を定義し、
`CMakeLists.txt` の `add_unity_test()` で全テストターゲットにリンク。

**次回のために:**
CMake で Unity を使うときは必ず `test_common.c` 等にスタブを用意すること。
PlatformIO では不要だが CMake では必須。

---

### 3. GitLab CI サブモジュール SSH クローンエラー

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

### 4. Riceエンコーダのランレングス遷移バグ

**うまくいかなかったこと:**
Rice エンコーダで `bit != currentBit` の遷移時に `runLen=0` にリセットしていた。
遷移点のピクセル (新しいランの先頭) が新ランにカウントされず、
「runLen=0」として書かれると decoder が「0ピクセルで flip」と解釈し、
written が進まないまま次の行のデータを読んでしまうバグ。

**うまくいったこと:**
遷移時に `runLen = (x < width) ? 1 : 0` として現在ピクセルを新ランに含める。

**次回のために:**
RLE 実装では遷移時に「前のランを書いた後、現在ピクセルは新ランの1個目」になることを確認すること。

---

### 5. テストデータの FileHeader サイズ不足

**うまくいかなかったこと:**
`test_codec_invalid_signature/version` のテストデータが14バイトだった。
`FileHeader` は signature(4)+version(1)+flags(1)+width(2)+height(2)+timebaseNum(2)+timebaseDen(2)+keyInterval(2) = 16バイト。
`readBytes(&_header, sizeof(_header))` が先にReadError(-3)を返してシグネチャチェックに到達しない。

**うまくいったこと:**
テストデータに `keyInterval` の 0, 0 を2バイト追加して16バイトにした。

**次回のために:**
FileHeader のサイズ変更時は必ずテストデータを更新すること。

---

### 6. RISC-V (ESP32C3) での std::max 型不一致エラー

**うまくいかなかったこと:**
`std::max(1u, _frequencies[...] / 2)` で `1u` は `unsigned int`、
`uint32_t` は RISC-V では `unsigned long` のため型が異なり、
テンプレートの型推論が失敗する。x86 (native/esp32dev) では同じ型なので通る。

**うまくいったこと:**
`std::max((uint32_t)1u, ...)` と明示キャストで解決。

**次回のために:**
`std::max` / `std::min` に整数リテラルを渡す場合は `(uint32_t)` 等で型を明示すること。
特に RISC-V ターゲットでは `uint32_t = unsigned long` になるため注意。

---

### 7. Rice大きいKでエンコード出力が黙って切り詰められる

**うまくいかなかったこと:**
`--rice-mode fixed --rice-k 7` で encode→decode が不一致（decode出力が空）。
原因は圧縮先バッファ `_frameBufferSize * 2 + 64` が不足。Rice(k=7)では
1ピクセルが独立ラン(長さ1)のとき (1+k)=8bit まで膨張し、ペイロードの約8倍になる。
`tmg1_mem_write` は溢れると黙って切り詰め、`RiceBitWriter::writeBit/writeSymbol` は
`writeByte` の失敗を無視するため、切り詰めが検出されずデコード不能になっていた。

**うまくいったこと:**
圧縮先バッファを `_frameBufferSize * 10 + height + 64` に拡大（k=7の最悪ケースに対応）。

**次回のために:**
Riceの圧縮先バッファはペイロード比で2倍では不足。最悪ケース(k=7)は約9倍。
根本的には writeBit 系が writeByte のエラーを伝播していないのが弱点（将来要改善）。

---

### 8. WSL を `bash -lc '...'` でインライン実行すると `$VAR`/`$(...)` が消える

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
スクリプトファイル化する。検証ハーネス(roundtrip.sh等)が既にこの形なのもそのため。
