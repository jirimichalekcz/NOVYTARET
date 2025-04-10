#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "globals.h"
#include "hx711_utils.h"
#include "utils.h"
#include "learning_utils.h"

//dosavadni verze stareho uceni 

// Dopředné deklarace, pokud nejsou v utils.h
void updateNextionText(String objectName, String text);
void hrajZvuk(int delka);

void uciciRezimServoA() {
  updateNextionText("status", "Učení servo A start");
  Serial.println("=== Učící režim servo A zahájen ===");

  Preferences flowPrefsA;
  flowPrefsA.begin("flowmapA", false); // Jiný namespace
  grafData.clear();

  int krok = 5; // Změna kroku na 5 stupňů
  float predchoziHmotnost = 0.0f;

  for (int relativeAngle = 0; relativeAngle <= 90; relativeAngle += krok) {
      int absoluteAngle = offsetServoA + relativeAngle;
      float rozdil = 0.0f;

      updateNextionText("status", "Úhel " + String(absoluteAngle));
      Serial.println("---------------");
      Serial.print("Úhel "); Serial.print(absoluteAngle); Serial.println("°, nové měření");

      tareScale();
      delay(500);
      zpracujHX711(); delay(200);
      float pred = currentWeight;

      servoA.write(absoluteAngle);
      delay(1000); // Servo zůstane otevřené 1 sekundu
      servoA.write(offsetServoA);

      delay(500);
      zpracujHX711(); delay(200);
      float po = currentWeight;

      rozdil = po - pred;
      Serial.print("Rozdíl: "); Serial.println(rozdil, 3);

      if (rozdil <= 0.2f) {
          Serial.println("!!! Úhel " + String(absoluteAngle) + " přeskočen (malý rozdíl)");
          updateNextionText("status", "Úhel " + String(absoluteAngle) + " přeskočen");
          delay(500);
          continue;
      }

      predchoziHmotnost = rozdil;

      String key = "A" + String(absoluteAngle);
      flowPrefsA.putFloat((key + "g").c_str(), rozdil);
      flowPrefsA.putUInt((key + "t").c_str(), 1000);
      grafData.push_back(rozdil);

      Serial.print("✔ Uloženo pro úhel "); Serial.print(absoluteAngle);
      Serial.print("°: "); Serial.print(rozdil, 3); Serial.println(" g");

      updateNextionText("status", "Úhel " + String(absoluteAngle) + " hotovo");
      delay(1000);
  }

  updateNextionText("status", "Učení servo A dokončeno");
  Serial.println("=== Učící režim dokončen ===");
  flowPrefsA.end();
}

void uciciRezimServoB() {
  updateNextionText("status", "Učení servo B start");
  Serial.println("=== Učící režim servo B zahájen ===");

  Preferences flowPrefsB;
  flowPrefsB.begin("flowmapB", false); // Namespace pro B
  grafData.clear();

  int krok = 5; // Změna kroku na 5 stupňů
  float predchoziHmotnost = 0.0f;

  for (int relativeAngle = 0; relativeAngle <= 90; relativeAngle += krok) {
      int absoluteAngle = offsetServoB + relativeAngle;
      float rozdil = 0.0f;

      updateNextionText("status", "Úhel " + String(absoluteAngle));
      Serial.println("---------------");
      Serial.print("Úhel "); Serial.print(absoluteAngle); Serial.println("°, nové měření");

      tareScale();
      delay(500);
      zpracujHX711(); delay(200);
      float pred = currentWeight;

      servoB.write(absoluteAngle);
      delay(1000); // Servo zůstane otevřené 1 sekundu
      servoB.write(offsetServoB);

      delay(500);
      zpracujHX711(); delay(200);
      float po = currentWeight;

      rozdil = po - pred;
      Serial.print("Rozdíl: "); Serial.println(rozdil, 3);

      if (rozdil <= 0.2f) {
          Serial.println("!!! Úhel " + String(absoluteAngle) + " přeskočen (malý rozdíl)");
          updateNextionText("status", "Úhel " + String(absoluteAngle) + " přeskočen");
          delay(500);
          continue;
      }

      predchoziHmotnost = rozdil;

      String key = "B" + String(absoluteAngle);
      flowPrefsB.putFloat((key + "g").c_str(), rozdil);
      flowPrefsB.putUInt((key + "t").c_str(), 1000);
      grafData.push_back(rozdil);

      Serial.print("✔ Uloženo pro úhel "); Serial.print(absoluteAngle);
      Serial.print("°: "); Serial.print(rozdil, 3); Serial.println(" g");

      updateNextionText("status", "Úhel " + String(absoluteAngle) + " hotovo");
      delay(1000);
  }

  updateNextionText("status", "Učení servo B dokončeno");
  Serial.println("=== Učící režim dokončen ===");
  flowPrefsB.end();
}

