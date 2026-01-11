#ifndef UNIT_TEST

#include <Arduino.h>
#include <LittleFS.h>
#include <U8g2lib.h>

#include "Tmg1Decoder.h"

// u8g2のコンストラクタ。指定されたドライバを使用します。
// U8G2_R0は回転なしを意味します。
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

Tmg1Decoder decoder;
uint8_t* frameBuffer = nullptr;
File videoFile;
const char* videoFileName = "/_bad-apple15.tmg1";

TickType_t xLastWakeTime;

// 動画情報をキャッシュする変数
uint16_t videoWidth = 0;
uint16_t videoHeight = 0;
size_t frameDataSize = 0;
uint16_t timebaseNum = 0;
uint16_t timebaseDen = 0;

void setup() {
  Serial.begin(115200);

  // u8g2の初期化
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Initializing...");
  u8g2.sendBuffer();

  // LittleFSの初期化
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system.");
    u8g2.drawStr(0, 22, "FS Mount Error!");
    u8g2.sendBuffer();
    while (1);  // エラーで停止
  }

  // 動画ファイルを開く
  videoFile = LittleFS.open(videoFileName, "r");
  if (!videoFile) {
    Serial.print("Failed to open video file: ");
    Serial.println(videoFileName);
    u8g2.drawStr(0, 22, "File Open Error!");
    u8g2.sendBuffer();
    while (1);  // エラーで停止
  }

  // デコーダーの初期化
  if (!decoder.begin(videoFile)) {
    Serial.println("Failed to initialize TMG1 decoder.");
    u8g2.drawStr(0, 22, "Decoder Error!");
    u8g2.sendBuffer();
    while (1);  // エラーで停止
  }

  // フレームバッファの確保
  // 動画情報を取得してグローバル変数にキャッシュ
  videoWidth = decoder.getWidth();
  videoHeight = decoder.getHeight();
  frameDataSize = (videoWidth * videoHeight + 7) / 8;
  timebaseNum = decoder.getTimebaseNum();
  timebaseDen = decoder.getTimebaseDen();

  frameBuffer = new (std::nothrow) uint8_t[frameDataSize];
  if (!frameBuffer) {
    Serial.println("Failed to allocate frame buffer.");
    u8g2.drawStr(0, 22, "Memory Error!");
    u8g2.sendBuffer();
    while (1);  // エラーで停止
  }

  Serial.println("Setup complete. Starting playback.");
  // FreeRTOSのタイマーを初期化
  xLastWakeTime = xTaskGetTickCount();
}

void loop() {
  // 1. 次のフレームをデコード
  if (decoder.decodeFrame(frameBuffer, frameDataSize)) {
    // 2. デコードしたフレームをu8g2のバッファに準備
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, videoWidth, videoHeight, frameBuffer);

    // 3. このフレームの表示時間を計算
    uint32_t ptsDelta = decoder.getLastPtsDelta();
    long frameDurationMillis = 33;  // デフォルト値
    if (timebaseDen > 0) {
      frameDurationMillis = (1000ULL * ptsDelta * timebaseNum) / timebaseDen;
    }
    const TickType_t xFrequency = pdMS_TO_TICKS(frameDurationMillis);

    // 4. 前回の描画時刻から、計算したフレーム時間分だけ待機
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // 5. 待機後、バッファの内容をディスプレイに転送
    u8g2.sendBuffer();
  } else {
    // 動画の終端に達したらストリームをリセットしてループ再生
    Serial.println("End of video. Restarting.");
    videoFile.seek(0);  // ファイルポインタを先頭に戻すだけでOK
    // デコーダーの内部状態（前のフレーム情報など）をリセットするためにbeginを再呼び出し
    if (!decoder.begin(videoFile)) {
      Serial.println("Failed to re-initialize decoder.");
      while (1);  // エラーで停止
    }
    delay(1000);
    xLastWakeTime = xTaskGetTickCount();  // タイマーをリセット
  }
}

#endif  // UNIT_TEST