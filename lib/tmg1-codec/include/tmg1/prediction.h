#pragma once
#include <stddef.h>
#include <stdint.h>

namespace tmg1 {

// 予測フィルタ (エンコーダ/デコーダ共通)
// バッファはビットパックされた1bppデータ (1バイト = 8ピクセル)

// 順方向予測を適用する (エンコード時)
// predictionMethod: PREDICTION_* 定数
void applyPrediction(uint8_t* buffer, size_t bufferSize,
                     uint16_t width, uint16_t height,
                     uint8_t predictionMethod);

// 逆予測を適用する (デコード時)
void applyInversePrediction(uint8_t* buffer, size_t bufferSize,
                            uint16_t width, uint16_t height,
                            uint8_t predictionMethod);

} // namespace tmg1
