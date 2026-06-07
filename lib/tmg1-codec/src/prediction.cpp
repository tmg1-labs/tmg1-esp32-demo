#include "tmg1/prediction.h"
#include "tmg1/types.h"

namespace tmg1 {

void applyPrediction(uint8_t* buffer, size_t bufferSize,
                     uint16_t width, uint16_t height,
                     uint8_t predictionMethod)
{
    if (predictionMethod == PREDICTION_NONE) return;

    size_t bytesPerLine = (width + 7) / 8;

    if (predictionMethod == PREDICTION_LEFT) {
        // 左方向差分: 各バイトを左隣との XOR にする (右から左へ処理)
        for (uint16_t y = 0; y < height; ++y) {
            uint8_t* line = buffer + y * bytesPerLine;
            for (size_t i = bytesPerLine - 1; i > 0; --i) {
                line[i] ^= line[i - 1];
            }
        }
    } else if (predictionMethod == PREDICTION_UP) {
        // 上方向差分: 各バイトを上の行との XOR にする (下から上へ処理)
        for (uint16_t y = height - 1; y > 0; --y) {
            uint8_t* cur   = buffer + y * bytesPerLine;
            uint8_t* upper = buffer + (y - 1) * bytesPerLine;
            for (size_t i = 0; i < bytesPerLine; ++i) {
                cur[i] ^= upper[i];
            }
        }
    }
}

void applyInversePrediction(uint8_t* buffer, size_t bufferSize,
                            uint16_t width, uint16_t height,
                            uint8_t predictionMethod)
{
    if (predictionMethod == PREDICTION_NONE) return;

    size_t bytesPerLine = (width + 7) / 8;

    if (predictionMethod == PREDICTION_LEFT) {
        // 左方向逆差分: 各バイトに左隣を XOR して復元 (左から右へ処理)
        for (uint16_t y = 0; y < height; ++y) {
            uint8_t* line = buffer + y * bytesPerLine;
            for (size_t i = 1; i < bytesPerLine; ++i) {
                line[i] ^= line[i - 1];
            }
        }
    } else if (predictionMethod == PREDICTION_UP) {
        // 上方向逆差分: 各バイトに上の行を XOR して復元 (上から下へ処理)
        for (uint16_t y = 1; y < height; ++y) {
            uint8_t* cur   = buffer + y * bytesPerLine;
            uint8_t* upper = buffer + (y - 1) * bytesPerLine;
            for (size_t i = 0; i < bytesPerLine; ++i) {
                cur[i] ^= upper[i];
            }
        }
    }
}

} // namespace tmg1
