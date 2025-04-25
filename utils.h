#pragma once

#include <Arduino.h>
#include "config.h"

void setupIndicators(int ledPin, int buzzerPin);
void turnLight(bool status);
void infoIndicator(const String& message, bool shouldLight = false, bool shouldBuzz = false);
void errorIndicator(const String& message, bool shouldBuzz = false);
