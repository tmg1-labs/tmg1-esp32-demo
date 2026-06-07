#pragma once
#include <stdint.h>

namespace tmg1 {

// フォーマットバージョン (v2: 32bit Rangeコーダ)
static const uint8_t TMG1_VERSION = 2;

// シグネチャ 'TMG1' (リトルエンディアン)
static const uint32_t TMG1_SIGNATURE = 0x31474D54u;

// エラーコード
enum class Error : int {
    None             =  0,
    InvalidSignature = -1,
    InvalidVersion   = -2,
    ReadError        = -3,
    WriteError       = -4,
    BufferTooSmall   = -5,
    InvalidData      = -6,
};

// ファイルフラグ (fileHeader.flags)
static const uint8_t FLAG_MSB_FIRST   = 0x01; // ビット順: MSBファースト
static const uint8_t FLAG_RANGE_CODER = 0x04; // Rangeコーダ使用 (0=Rice)

// フレームタイプ
static const uint8_t FRAME_TYPE_I = 0; // キーフレーム
static const uint8_t FRAME_TYPE_P = 1; // 予測フレーム

// フレームフラグ (frameHeader.frameFlags)
static const uint8_t FRAME_FLAG_START_BIT = 0x01; // 行頭スタートビット
static const uint8_t FRAME_FLAG_PER_LINE_K  = 0x02; // 行ごとのKパラメータ
static const uint8_t FRAME_FLAG_PER_FRAME_K = 0x04; // フレームごとのKパラメータ

// 予測方式
static const uint8_t PREDICTION_NONE = 0; // 予測なし
static const uint8_t PREDICTION_LEFT = 1; // 左方向予測
static const uint8_t PREDICTION_UP   = 2; // 上方向予測

// ファイルヘッダ (16バイト固定)
#pragma pack(push, 1)
struct FileHeader {
    uint32_t signature;   // 'TMG1' = 0x31474D54
    uint8_t  version;     // フォーマットバージョン
    uint8_t  flags;       // ビット0: MSBファースト, ビット2: Rangeコーダ
    uint16_t width;       // 横幅 (ピクセル)
    uint16_t height;      // 縦幅 (ピクセル)
    uint16_t timebaseNum; // タイムベース分子
    uint16_t timebaseDen; // タイムベース分母
    uint16_t keyInterval; // キーフレーム間隔
};
#pragma pack(pop)

// フレームヘッダ (可変長: ULEB128エンコード使用)
struct FrameHeader {
    uint8_t  frameType;        // FRAME_TYPE_I / FRAME_TYPE_P
    uint32_t ptsDelta;         // PTS差分 (ULEB128)
    uint32_t payloadSize;      // ペイロードサイズ (ULEB128)
    uint8_t  frameFlags;       // FRAME_FLAG_* の組み合わせ
    uint8_t  predictionMethod; // PREDICTION_*
};

// エンコード設定
struct EncodeConfig {
    uint16_t width;
    uint16_t height;
    uint16_t timebaseNum;
    uint16_t timebaseDen;
    uint16_t keyInterval;   // 0: キーフレームのみ
    bool     msbFirst;      // true: MSBファースト
    bool     useRangeCoder; // true: Rangeコーダ, false: Riceコーダ
    bool     deltaEnabled;  // true: P-Frame有効
};

} // namespace tmg1
