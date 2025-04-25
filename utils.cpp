#include "utils.h"

namespace {
int buzzerPin_;
int ledPin_;
}

void setupIndicators(int ledPin, int buzzerPin) {
  buzzerPin_ = buzzerPin;
  ledPin_ = ledPin;

  pinMode(buzzerPin_, OUTPUT);
  pinMode(ledPin_, OUTPUT);
  neopixelWrite(ledPin_, 0, 0, 0);
}

void blinkLED(uint8_t r, uint8_t g, uint8_t b, int times) {
  for (int i = 0; i < times; ++i) {
    neopixelWrite(ledPin_, r, g, b);
    delay(200);
    neopixelWrite(ledPin_, 0, 0, 0);
    delay(200);
  }
}

void turnLight(bool status) {
  if (status) {
    neopixelWrite(ledPin_, 0, 255, 0);
  } else {
    neopixelWrite(ledPin_, 0, 0, 0);
  }
}

void setupLight(bool status) {
  if (status) {
    neopixelWrite(ledPin_, 255, 255, 0);
  } else {
    neopixelWrite(ledPin_, 0, 0, 0);
  }
}

void buzz(int times) {
  for (int i = 0; i < times; ++i) {
    tone(buzzerPin_, 1000, 100);
    delay(200);
  }
}

void infoIndicator(const String& message, bool shouldLight, bool shouldBuzz) {
#if DEBUG_MODE
  Serial.println("INFO: " + message);
#endif

  if (shouldLight) {
    blinkLED(0, 0, RGB_BRIGHTNESS, 1);
  }

  if (shouldBuzz) {
    buzz(1);
  }
}

void errorIndicator(const String& message, bool shouldBuzz) {
#if DEBUG_MODE
  Serial.println("ERROR: " + message);
#endif
  blinkLED(RGB_BRIGHTNESS, 0, 0, 1);

  if (shouldBuzz) {
    buzz(2);
  }
}
