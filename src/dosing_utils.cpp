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



void davkujSlozkuAUcenim(float cilovaHmotnost) {
  Preferences prefsA;
  prefsA.begin("flowmapA", true); // správný namespace pro servo A
  tareScale();
  delay(500);
  float hmotnostDosud = 0.0f;
  currentWeight = 0.0f;
  grafData.clear();

  String csvLog = "";  // CSV výstup



  while (hmotnostDosud < cilovaHmotnost) {
    float zbyva = cilovaHmotnost - hmotnostDosud;
    int vhodnyUhel = 0;

    for (int angle = 5; angle <= 90; angle++) {
      String key = "A" + String(angle);
      float g = prefsA.getFloat((key + "g").c_str(), 0.0f);
      if (g > 0.01f && g <= zbyva) {
        vhodnyUhel = angle;
      }
    }

    if (vhodnyUhel == 0) {
      Serial.println("Nelze dávkovat přesněji, zůstává: " + String(zbyva, 3) + " g");
      prefsA.end(); // nezapomeň ukončit
      break;
    }

    String key = "A" + String(vhodnyUhel);
    unsigned int cas = prefsA.getUInt((key + "t").c_str(), 1000);

    Serial.print("Dávkuji: "); Serial.print(vhodnyUhel);
    Serial.print("°, čas: "); Serial.print(cas);
    Serial.print(" ms, offset: "); Serial.println(offsetServoA);

    servoA.write(offsetServoA + vhodnyUhel);
    delay(cas);
    servoA.write(offsetServoA);

    delay(2000);
    zpracujHX711();
    float novaHmotnost = currentWeight;
    float prirustek = novaHmotnost - hmotnostDosud;
    hmotnostDosud = novaHmotnost;
    grafData.push_back(hmotnostDosud);
    csvLog += String(vhodnyUhel) + "," + String(prirustek, 3) + "; ";


    updateNextionText("currentWA", String(hmotnostDosud, 2));
  }


Serial.print("=== CSV DAVKOVANI A / ");
Serial.print(cilovaHmotnost, 1);
Serial.println(" g ===");
Serial.println(csvLog);
Serial.println("=== Konec CSV výpisu ===");


  prefsA.end();
  updateNextionText("status", "Dávkování A dokončeno");
  hrajZvuk(600);
}




void davkujSlozkuBUcenim(float cilovaHmotnost) {
  Preferences prefsB;
  prefsB.begin("flowmapB", true); // správný namespace pro servo B
  tareScale();
  delay(500);
  float hmotnostDosud = 0.0f;
  currentWeight = 0.0f;
  grafData.clear();

  String csvLog = "";  // CSV výstup


  while (hmotnostDosud < cilovaHmotnost) {
    float zbyva = cilovaHmotnost - hmotnostDosud;
    int vhodnyUhel = 0;

    for (int angle = 5; angle <= 90; angle++) {
      String key = "B" + String(angle);
      float g = prefsB.getFloat((key + "g").c_str(), 0.0f);
      if (g > 0.01f && g <= zbyva) {
        vhodnyUhel = angle;
      }
    }

    if (vhodnyUhel == 0) {
      Serial.println("Nelze dávkovat přesněji, zůstává: " + String(zbyva, 3) + " g");
      prefsB.end(); // nezapomeň ukončit
      break;
    }

    String key = "B" + String(vhodnyUhel);
    unsigned int cas = prefsB.getUInt((key + "t").c_str(), 1000);

    Serial.print("Dávkuji: "); Serial.print(vhodnyUhel);
    Serial.print("°, čas: "); Serial.print(cas);
    Serial.print(" ms, offset: "); Serial.println(offsetServoB);

    servoB.write(offsetServoB + vhodnyUhel);
    delay(cas);
    servoB.write(offsetServoB);

    delay(2000);
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

