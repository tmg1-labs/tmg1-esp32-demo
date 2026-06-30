# tmg1-esp32-demo

[English](README.md) | **日本語**

**TMG1**（1ピクセル1ビット＝ビットプレーン）動画フォーマット向けの、
**ESP32 / Arduino** リファレンスプレイヤーです。共有デコーダライブラリ
[`tmg1-codec`](https://github.com/tmg1-labs/tmg1-codec) をデバイス上で使う方法を
示します。LittleFS 上の `.tmg1` ファイルをデコードし、
[U8g2](https://github.com/olikraus/u8g2) 経由で SSD1306 128x64 I2C OLED に
表示します。

デコード処理そのものは**このリポジトリには実装されていません**。本体は
`tmg1-codec` にあり、これ自体が独立した Arduino ライブラリです
（`library.properties` と `tmg1/arduino_stream.h` アダプタを同梱）。本リポジトリは
その**サンプル**で、[src/main.cpp](src/main.cpp) がサンプルスケッチ、codec は
サンプルがビルドできるようサブモジュールとして同梱しているだけです。エンコードに
使った codec コミットにサブモジュールで固定されるため、デバイス側デコーダの
フォーマット解釈は常にエンコード時と一致します。

## 自分のプロジェクトで codec を使う

自分の Arduino / PlatformIO スケッチで TMG1 をデコードするには、本リポジトリでは
なく **`tmg1-codec`** に依存します。PlatformIO では `lib_deps` に追加します:

```ini
lib_deps =
    https://github.com/tmg1-labs/tmg1-codec.git
    olikraus/U8g2@^2.36.15   ; U8g2 ディスプレイに描画する場合のみ
```

あとは任意の Arduino `Stream` / `File` を `Tmg1Stream` にラップし、フレームを
1 つずつ取り出します:

```cpp
#include <LittleFS.h>
#include "tmg1/decoder.h"
#include "tmg1/arduino_stream.h"

tmg1::Decoder decoder;

void setup() {
  LittleFS.begin();
  File f = LittleFS.open("/video.tmg1", "r");

  // Arduino File を codec ストリームにラップしてヘッダを読む
  Tmg1Stream stream = tmg1_stream_from_arduino(f);
  decoder.begin(stream);

  const size_t frameSize = (decoder.getWidth() * decoder.getHeight() + 7) / 8;
  uint8_t* buf = new uint8_t[frameSize];

  // 呼び出しごとに1フレーム: ceil(w*h/8) バイトの MSBファースト 1bpp ビットプレーン
  while (decoder.decodeFrame(buf, frameSize) == tmg1::Error::None) {
    // ... buf を描画し、decoder.getLastPtsDelta() とタイムベースで間隔調整 ...
  }
}
```

完全版（OLED 出力・FPS オーバーレイ・マイクロ秒精度のフレーム間隔制御・ループ）は
[src/main.cpp](src/main.cpp) を参照してください。動画から `.tmg1` を生成するには
[`tmg1-cli`](https://github.com/tmg1-labs/tmg1-cli) を使います。

## サンプルプレイヤー

[src/main.cpp](src/main.cpp) は完全なプレイヤーの実装例です:

- **ストリーミングデコード** — フレームは LittleFS から逐次読み出し、`Tmg1Stream`
  を通して 1 フレームずつデコードします。動画全体を RAM に載せる必要はありません。
- **OLED 表示 + FPS オーバーレイ** — デコード済みビットプレーンを `u8g2.drawXBMP`
  で描画し、隅に毎秒フレーム数（FPS）をライブ表示します。
- **VFR 対応のタイミング** — 各フレームの再生間隔をストリームの `ptsDelta` と
  タイムベースからマイクロ秒精度で算出するため、可変フレームレート（VFR）でも
  正しい速度で再生されます。
- **ループ再生** — ストリーム終端でファイルを再オープンし、最初から再生し直します。

### ハードウェア

- ESP32 / ESP32-C3 系ボード。
- SSD1306 128x64 I2C OLED。

> **表示極性:** 対象 OLED では `drawXBMP` がビット `1` を描画色として扱うため、
> 画面の白黒が反転して表示されます。そのためサンプルは、動画がエンコード時に
> 色反転（ffmpeg `negate`）してある前提とし、コーデック／デコーダは無改造のままです。
> 詳細は [src/main.cpp](src/main.cpp) のコメントを参照してください。

### ビルド

```bash
git clone --recursive https://github.com/tmg1-labs/tmg1-esp32-demo
cd tmg1-esp32-demo
pio run -e esp32dev            # ESP32 DevKit
pio run -e seeed_xiao-esp32c3  # Seeed XIAO ESP32-C3
pio run -e super-mini-k2       # Super Mini K2 (ESP32-C3)
```

必要なもの:

- [PlatformIO](https://platformio.org/)（`pip install platformio`）。
- `tmg1-codec` サブモジュール。`--recursive` を付け忘れた場合は
  `git submodule update --init --recursive` で取得します。

### アップロード / 書き込み

```bash
pio run -e esp32dev -t upload     # ファームウェアを書き込む
pio run -e esp32dev -t uploadfs   # data/ から生成した LittleFS イメージを書き込む
pio device monitor -b 115200      # シリアルモニタ
```

`.tmg1` ファイルは `data/` に置きます。サンプルは LittleFS 上の
`/sample-video.tmg1` を再生する（[src/main.cpp](src/main.cpp) 参照）ため、その名前で
配置するか `videoFileName` を変更してください。

## テスト

```bash
# PlatformIO native テスト
pio test -e native -v

# tmg1-codec 単体テスト（CMake + Unity）
cmake -B build -S lib/tmg1-codec -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && ctest --output-on-failure
```

## ビルドと CI

CI は GitLab（`.gitlab-ci.yml`）で 2 ジョブを実行します: `test_native`
（PlatformIO native テスト）と `test_cmake`（codec の ctest）。サブモジュールは
`GIT_SUBMODULE_STRATEGY: recursive` で取得します。

## TMG1 フォーマット

`.tmg1` のバイト単位の正確なレイアウトは、独立した
[**TMG1 フォーマット仕様書**](https://github.com/tmg1-labs/.github/blob/main/docs/tmg1-format.ja.md)
に置きます。コーデック内部や C++/FFI API は
[`tmg1-codec`](https://github.com/tmg1-labs/tmg1-codec) を、デスクトップでの
`.tmg1` 生成は [`tmg1-cli`](https://github.com/tmg1-labs/tmg1-cli) を参照してください。

## 関連プロジェクト

**[TMG1 Labs](https://github.com/tmg1-labs)** の一部です。プロジェクトの全リポジトリ
一覧は組織プロフィールを参照してください。

## ライセンス

MIT
