#include <Arduino.h>
#include <LittleFS.h>
#include <U8g2lib.h>
#include "Tmg1Decoder.h"

// U8g2 Constructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

Tmg1Decoder decoder;
uint8_t* frameBuffer = nullptr;
File videoFile;
const char* videoFileName = "/bad-apple15.tmg1";

// Microsecond precision timing
uint32_t nextFrameMicros = 0;

// Video Metadata
uint16_t videoWidth = 0;
uint16_t videoHeight = 0;
size_t frameDataSize = 0;
uint16_t timebaseNum = 0;
uint16_t timebaseDen = 0;

// FPS Calculation
uint32_t lastFpsTime = 0;
uint32_t frameCount = 0;
float currentFps = 0.0;

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

  videoFile = LittleFS.open(videoFileName, "r");
  if (!videoFile) {
    Serial.print("Failed to open video file: ");
    Serial.println(videoFileName);
    u8g2.drawStr(0, 22, "File Open Error!");
    u8g2.sendBuffer();
    while (1);
  }

  if (!decoder.begin(videoFile)) {
    Serial.println("Failed to initialize TMG1 decoder.");
    u8g2.drawStr(0, 22, "Decoder Error!");
    u8g2.sendBuffer();
    while (1);
  }

  decoder.setReverseBitOrder(true);

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
    while (1);
  }

  Serial.println("Setup complete. Starting playback.");
  
  lastFpsTime = millis();
  // Initialize timing
  nextFrameMicros = micros();
}

void loop() {
  if (decoder.decodeFrame(frameBuffer, frameDataSize)) {
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, videoWidth, videoHeight, frameBuffer);

    // FPS Counting
    frameCount++;
    uint32_t now = millis();
    if (now - lastFpsTime >= 1000) {
      currentFps = (float)frameCount * 1000.0f / (now - lastFpsTime);
      frameCount = 0;
      lastFpsTime = now;
    }

    // FPS Display
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, 25, 9);
    u8g2.setDrawColor(1);
    u8g2.setCursor(1, 7);
    u8g2.print(currentFps, 1);

    // Timing Logic (Microsecond precision)
    uint32_t ptsDelta = decoder.getLastPtsDelta();
    uint32_t frameDurationUs = 0;
    if (timebaseDen > 0) {
      frameDurationUs = (uint32_t)((1000000ULL * ptsDelta * timebaseNum) / timebaseDen);
    }

    // Schedule next frame
    nextFrameMicros += frameDurationUs;

    // Wait until schedule
    long timeToWait = (long)(nextFrameMicros - micros());
    
    if (timeToWait > 0) {
      // If wait is long enough (>2ms), yield to CPU (allow background tasks)
      if (timeToWait > 2000) {
        delay(timeToWait / 1000 - 1);
      }
      // Busy wait for the remaining time for precision
      while ((long)(nextFrameMicros - micros()) > 0) {
         __asm__ __volatile__("nop");
      }
    } else {
      // We are late
      // If extremely late (>100ms), reset sync to prevent burst playback
      if (timeToWait < -100000) {
         nextFrameMicros = micros();
      }
    }

    // Update display (Send I2C)
    // We send AFTER waiting, to ensure the interval between Display Updates is periodic.
    u8g2.sendBuffer();

  } else {
    Serial.println("End of video. Restarting.");
    videoFile.seek(0);
    if (!decoder.begin(videoFile)) {
      Serial.println("Failed to re-initialize decoder.");
      while (1);
    }
    
    // Reset Timing
    delay(1000); 
    nextFrameMicros = micros();
    lastFpsTime = millis();
  }
}