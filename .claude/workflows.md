# よく使うコマンド・手順

## セットアップ
```bash
# サブモジュール込みでクローン / 取得
git submodule update --init --recursive
# PlatformIO (pip 経由)
pip install platformio
```

## ビルド（ファームウェア）
```bash
pio run -e esp32dev              # ESP32 DevKit
pio run -e seeed_xiao-esp32c3    # XIAO ESP32-C3
pio run -e super-mini-k2         # Super Mini K2 (C3)
```

## アップロード / シリアルモニタ
```bash
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs  # LittleFS イメージ (data/) を書き込み
pio device monitor -b 115200
```

## テスト
```bash
# PlatformIO native テスト
pio test -e native -v

# コーデック単体 (CMake + Unity)
cmake -B build -S lib/tmg1-codec -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)
cd build && ctest --output-on-failure
```
- CMake テストは Unity を `lib/tmg1-codec/vendor/unity` に clone して使う（CI 参照）。

## CI
- `.gitlab-ci.yml`: `test_native`(pio) と `test_cmake`(ctest) の 2 ジョブ。
- 実行条件: MR / デフォルトブランチ / `feature/*` ブランチ。
- サブモジュールは `GIT_SUBMODULE_STRATEGY: recursive`、相対 URL で取得。

## codec サブモジュールの同期
1. `lib/tmg1-codec` 本流（別リポジトリ）で修正・push。
2. このリポジトリで `lib/tmg1-codec` のポインタを更新し、コミット（CLI 側も同様に同期）。

## 動画 → tmg1 変換
- ffmpeg パイプライン + Rust CLI(`tmg1-cli`)。決定経緯は `.claude/context/session-history.md` を参照。
