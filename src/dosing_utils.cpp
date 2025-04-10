#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <Q2HX711.h>
#include <ESP32Servo.h>
#include "globals.h"
#include "hx711_utils.h"
#include "utils.h"

//odmazan pokus  a prepsan radek v dosing_utis

void updateNextionText(String objectName, String text);

  Preferences prefsB;
  prefsB.begin("flowmapB", true); // Namespace pro servo B
  tareScale();
  delay(500);
  float hmotnostDosud = 0.0f;
  currentWeight = 0.0f;
  grafData.clear();

  String csvLog = ""; // CSV výstup

  while (hmotnostDosud < cilovaHmotnost) {
      float zbyva = cilovaHmotnost - hmotnostDosud;
      int vhodnyUhel = 0;

      // Vyhledání vhodného úhlu pro dávkování
      for (int angle = 5; angle <= 90; angle += 5) { // Krok 5 stupňů
          String key = "B" + String(angle);
          float g = prefsB.getFloat((key + "g").c_str(), 0.0f);
          if (g > 0.01f && g <= zbyva) {
              vhodnyUhel = angle;
          }
      }

      if (vhodnyUhel == 0) {
          Serial.println("Nelze dávkovat přesněji, zůstává: " + String(zbyva, 3) + " g");
          prefsB.end(); // Ukončení práce s NVS
          break;
      }

      String key = "B" + String(vhodnyUhel);
      unsigned int cas = prefsB.getUInt((key + "t").c_str(), 1000);

      Serial.print("Dávkuji: "); Serial.print(vhodnyUhel);
      Serial.print("°, čas: "); Serial.print(cas);
      Serial.print(" ms, offset: "); Serial.println(offsetServoB);

      // Nastavení serva a dávkování
      servoB.write(offsetServoB + vhodnyUhel);
      delay(cas);
      servoB.write(offsetServoB);

      delay(2000); // Čekání na stabilizaci váhy
      zpracujHX711();
      float novaHmotnost = currentWeight;
      float prirustek = novaHmotnost - hmotnostDosud;
      hmotnostDosud = novaHmotnost;
      grafData.push_back(hmotnostDosud);
      csvLog += String(vhodnyUhel) + "," + String(prirustek, 3) + "; ";

      updateNextionText("currentWB", String(hmotnostDosud, 2));
  }

  Serial.print("=== CSV DAVKOVANI B / ");
  Serial.print(cilovaHmotnost, 1);
  Serial.println(" g ===");
  Serial.println(csvLog);
  Serial.println("=== Konec CSV výpisu ===");

  prefsB.end();
  updateNextionText("status", "Dávkování B dokončeno");
  hrajZvuk(600);
}


void davkujSlozku(float cilovaHmotnost, Servo &servo, int offsetServo, const char *namespaceName) {
    Preferences prefs;
    prefs.begin(namespaceName, true); // Otevření namespace

    const int pocetUhlu = 19; // Pro úhly 0, 5, 10, ..., 90
    DosingData data[pocetUhlu];

    // Načtení dat z NVS
    size_t velikost = prefs.getBytes("dosingData", data, sizeof(data));
    if (velikost != sizeof(data)) {
        Serial.println("Chyba: Data nebyla správně načtena z NVS.");
        prefs.end();
        return;
    }

    float hmotnostDosud = 0.0f;
    while (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        int vhodnyUhel = -1;

        // Vyhledání vhodného úhlu
        for (int i = 0; i < pocetUhlu; i++) {
            if (data[i].hmotnost > 0.01f && data[i].hmotnost <= zbyva) {
                vhodnyUhel = data[i].uhel;
                break;
            }
        }

        if (vhodnyUhel == -1) {
            Serial.println("Nelze dávkovat přesněji, zůstává: " + String(zbyva, 3) + " g");
            break;
        }

        Serial.print("Dávkuji: "); Serial.print(vhodnyUhel);
        Serial.print("°, hmotnost: "); Serial.println(data[vhodnyUhel / 5].hmotnost);

        // Nastavení serva a dávkování
        servo.write(offsetServo + vhodnyUhel);
        delay(1000); // Konstantní čas dávkování
        servo.write(offsetServo);

        delay(2000); // Čekání na stabilizaci váhy
        zpracujHX711();
        float novaHmotnost = currentWeight;
        hmotnostDosud += novaHmotnost;

        updateNextionText("currentW", String(hmotnostDosud, 2));
    }

    prefs.end();
    updateNextionText("status", String("Dávkování ") + namespaceName + " dokončeno");
    Serial.println("=== Dávkování dokončeno ===");
}