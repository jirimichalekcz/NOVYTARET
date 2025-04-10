#include "hx711_utils.h"
#include "globals.h"
#include "utils.h"

#include <Arduino.h>

void updateNextionText(String objectName, String text);


// Časové proměnné a surová data z váhy
static unsigned long lastReadTime = 0;
const unsigned long readInterval = 200;

static long jaRaw = 0;
static long naRaw = 0;
long prumerRaw = 0;
static long rozdil = 0;

static float lastWeightForWatchdog = 0.0f;
static unsigned long lastWeightChangeTime = 0;
const unsigned long WATCHDOG_TIMEOUT = 10000; // 10 sekund

// === Funkce ===

void tareScale() {
  long sum = 0;
  const int readings = 10;
  for (int i = 0; i < readings; i++) {
    sum += hx711.read();
    delay(10);
  }
  offset = sum / readings;
  Serial.print("Offset po tarování nastaven na: ");
  Serial.println(offset);
  updateNextionText("offset", String(offset));
}

void zpracujHX711() {
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    jaRaw = hx711.read() / 100;
    naRaw = hx711.read() / 100;

    rozdil = abs(jaRaw - naRaw);
    if (rozdil <= 100) {
      prumerRaw = (jaRaw + naRaw) / 2;
      currentWeight = (prumerRaw - (offset / 100)) / calibrationFactor;

      updateNextionText("currentWeight", String(currentWeight, 1));
      updateNextionText("jaRaw", String(jaRaw));
      updateNextionText("naRaw", String(naRaw));
      updateNextionText("prumerRaw", String(prumerRaw));
      updateNextionText("rozdil", String(rozdil));

      if (abs(currentWeight - lastWeightForWatchdog) >= 0.1f) {
        lastWeightForWatchdog = currentWeight;
        lastWeightChangeTime = millis();
      }

    } else {
      Serial.println("Rozdíl mezi měřeními je příliš velký.");
      updateNextionText("status", "Measurement ...");
    }
  }
}

void vypisHmotnost(float hmotnost) {
  Serial.print("Hmotnost: ");
  Serial.print(hmotnost, 1);
  Serial.println(" g");
  updateNextionText("weight", String(hmotnost, 1));
}

void kontrolujWatchdog() {
  if (dosingMode != NONE && currentState != WAITING_FOR_INPUT) {
    if (millis() - lastWeightChangeTime > WATCHDOG_TIMEOUT) {
      Serial.println("WATCHDOG: Hmotnost se nezměnila 10 sekund! Přerušení dávkování.");

      if (servoAOpened) {
        servoA.write(offsetServoA);
        servoAOpened = false;
      }
      if (servoBOpened) {
        servoB.write(offsetServoB);
        servoBOpened = false;
      }

      updateNextionText("status", "ERROR: Flow stopped");
      hrajVarovnyZvuk();

      dosingMode = NONE;
      currentState = WAITING_FOR_INPUT;
    }
  }
}
