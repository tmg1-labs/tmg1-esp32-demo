# 現在の作業コンテキスト

最終更新: 2026-06-28

## 今やっていること
- 実機 OLED 向けの動画素材の調整（ffmpeg パイプライン）。表示極性は XOR 反転で対処（2026-06-28 burenai 実機 OK）。
- `tmg1-cli` を dotnet 版 CLI と機能パリティにする作業はほぼ完了
  （prediction / rice-mode・rice-k / scd / vfr / index / info+transcode を実装済み）。

## 直近の未確定・未完了
- `transcode` は ffmpeg 未導入のため e2e 未検証（実装は完了）。
- `info` 詳細化 + `transcode` のコミットは未実施の可能性あり（push 状況は要確認）。

## 一時的な制約・注意事項
- `data/` 配下のサンプル `.tmg1` は再エンコードで差し替えることがある（git status 参照）。
- standalone の `d:/workspace/TsuMuGi/tmg1-codec` 別クローンはビルド非依存・放置可（古いコミット）。

## 次にやること
- 残作業の push 状況確認と、codec サブモジュールポインタの同期。
- 実機での表示確認とサンプル動画の最終調整。

## 参考
- 決定経緯・セッション履歴の詳細は `session-history.md`、過去のエラー解決は `known-issues.md` / `errors-log.md`。
