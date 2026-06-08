#include <Arduino.h>
#include <LittleFS.h>
#include <U8g2lib.h>
#include <new>
#include "tmg1/decoder.h"
#include "tmg1/arduino_stream.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

tmg1::Decoder decoder;
uint8_t* frameBuffer      = nullptr;
File     videoFile;
const char* videoFileName = "/bad-apple15.tmg1";

uint32_t nextFrameMicros = 0;

uint16_t videoWidth    = 0;
uint16_t videoHeight   = 0;
size_t   frameDataSize = 0;
uint16_t timebaseNum   = 0;
uint16_t timebaseDen   = 0;

uint32_t lastFpsTime = 0;
uint32_t frameCount  = 0;
float    currentFps  = 0.0f;

// ファイルを開いてデコーダを初期化する。失敗時は停止。
static void openAndInitDecoder() {
    videoFile = LittleFS.open(videoFileName, "r");
    if (!videoFile) {
        Serial.print("Failed to open: ");
        Serial.println(videoFileName);
        u8g2.drawStr(0, 22, "File Open Error!");
        u8g2.sendBuffer();
        while (1);
    }

    // Arduino File を Tmg1Stream にラップ
    Tmg1Stream stream = tmg1_stream_from_arduino(videoFile);
    tmg1::Error err = decoder.begin(stream);
    if (err != tmg1::Error::None) {
        Serial.print("Decoder begin error: ");
        Serial.println((int)err);
        u8g2.drawStr(0, 22, "Decoder Error!");
        u8g2.sendBuffer();
        while (1);
    }
}

void setup() {
    Serial.begin(115200);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Initializing...");
    u8g2.sendBuffer();

    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system.");
        u8g2.drawStr(0, 22, "FS Mount Error!");
        u8g2.sendBuffer();
        while (1);
    }

    openAndInitDecoder();

    videoWidth    = decoder.getWidth();
    videoHeight   = decoder.getHeight();
    frameDataSize = (size_t)(videoWidth * videoHeight + 7) / 8;
    timebaseNum   = decoder.getTimebaseNum();
    timebaseDen   = decoder.getTimebaseDen();

    frameBuffer = new (std::nothrow) uint8_t[frameDataSize];
    if (!frameBuffer) {
        Serial.println("Memory allocation failed.");
        u8g2.drawStr(0, 22, "Memory Error!");
        u8g2.sendBuffer();
        while (1);
    }

    Serial.println("Setup complete. Starting playback.");
    lastFpsTime    = millis();
    nextFrameMicros = micros();
}

void loop() {
    tmg1::Error err = decoder.decodeFrame(frameBuffer, frameDataSize);
    if (err == tmg1::Error::None) {
        u8g2.clearBuffer();
        // drawXBMP はXBM形式(LSBファースト)を期待する。
        // デコード済みバッファはMSBファーストのため、各バイトのビット順を反転して渡す。
        for (size_t i = 0; i < frameDataSize; ++i) {
            uint8_t b = frameBuffer[i];
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            frameBuffer[i] = b;
        }
        u8g2.drawXBMP(0, 0, videoWidth, videoHeight, frameBuffer);

        // FPS 計算
        frameCount++;
        uint32_t now = millis();
        if (now - lastFpsTime >= 1000) {
            currentFps  = (float)frameCount * 1000.0f / (float)(now - lastFpsTime);
            frameCount  = 0;
            lastFpsTime = now;
        }

        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 0, 25, 9);
        u8g2.setDrawColor(1);
        u8g2.setCursor(1, 7);
        u8g2.print(currentFps, 1);

        // フレーム間隔管理 (マイクロ秒精度)
        uint32_t ptsDelta = decoder.getLastPtsDelta();
        uint32_t frameDurationUs = 0;
        if (timebaseDen > 0) {
            frameDurationUs = (uint32_t)((1000000ULL * ptsDelta * timebaseNum) / timebaseDen);
        }
        nextFrameMicros += frameDurationUs;

        long timeToWait = (long)(nextFrameMicros - micros());
        if (timeToWait > 0) {
            if (timeToWait > 2000) delay(timeToWait / 1000 - 1);
            while ((long)(nextFrameMicros - micros()) > 0) {
                __asm__ __volatile__("nop");
            }
        } else if (timeToWait < -100000) {
            nextFrameMicros = micros(); // 大幅に遅延した場合は同期リセット
        }

        u8g2.sendBuffer();
    } else {
        Serial.println("End of video or decode error. Restarting.");
        videoFile.close();
        openAndInitDecoder();
        delay(1000);
        nextFrameMicros = micros();
        lastFpsTime     = millis();
    }
}
