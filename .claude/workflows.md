# よく使うコマンド・手順

## セットアップ
```bash
# PlatformIO (pip 経由)。codec は lib_deps(git タグ) で自動取得されるためサブモジュール不要。
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
# PlatformIO native テスト（codec は lib_deps で自動取得される）
pio test -e native -v
```
- codec 単体の CMake + Unity テストは codec 本体リポジトリ(`tmg1-labs/tmg1-codec`)の CI が担う。
  本リポジトリでは重複させない。

## CI
- `.github/workflows/ci.yml`（GitHub Actions、`ubuntu-latest`）: `test_native`(pio) の 1 ジョブ。
- 実行条件: `push`（`main` / `feature/**` ブランチ）と `pull_request`。
- codec は PlatformIO の `lib_deps`（`https://github.com/tmg1-labs/tmg1-codec.git#v0.2.0`）で取得するため
  サブモジュール init は不要。

## codec の同期（lib_deps）
1. `tmg1-codec` 本流（別リポジトリ）で修正・push し、`git tag vX.Y.Z` を切って push。
2. このリポジトリの `platformio.ini` 各 env の `lib_deps` の `#vX.Y.Z` を上げてコミット（CLI 側も同様に同期）。

## 動画 → tmg1 変換
- ffmpeg パイプライン + Rust CLI(`tmg1-cli`)。決定経緯は `.claude/context/session-history.md` を参照。
