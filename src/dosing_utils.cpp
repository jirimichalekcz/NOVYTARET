#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <Q2HX711.h>
#include <ESP32Servo.h>
#include "globals.h"
#include "hx711_utils.h"
#include "utils.h"

void updateNextionText(String objectName, String text);

// Dopredna deklarace funkce davkujSlozku
void davkujSlozku(float cilovaHmotnost, Servo &servo, int offsetServo, const char *namespaceName);

void davkujSlozkuAUcenim(float cilovaHmotnost) {
    davkujSlozku(cilovaHmotnost, servoA, offsetServoA, "flowmapA");
}

void davkujSlozkuBUcenim(float cilovaHmotnost) {
    davkujSlozku(cilovaHmotnost, servoB, offsetServoB, "flowmapB");
}

void davkujSlozku(float cilovaHmotnost, Servo &servo, int offsetServo, const char *namespaceName) {
    Preferences prefs;
    prefs.begin(namespaceName, true); // Otevreni namespace

    const int pocetUhlu = 19; // Pro uhly 0, 5, 10, ..., 90
    DosingData data[pocetUhlu];

    // Nacteni dat z NVS
    size_t velikost = prefs.getBytes("dosingData", data, sizeof(data));
    if (velikost != sizeof(data)) {
        Serial.println("Chyba: Data nebyla spravne nactena z NVS.");
        prefs.end();
        return;
    }

    float hmotnostDosud = 0.0f;
    while (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        int vhodnyUhel = -1;

        // Vyhledani vhodneho uhlu
        for (int i = 0; i < pocetUhlu; i++) {
            if (data[i].hmotnost > 0.01f && data[i].hmotnost <= zbyva) {
                vhodnyUhel = data[i].uhel;
                break;
            }
        }

        if (vhodnyUhel == -1) {
            Serial.println("Nelze davkovat presneji, zustava: " + String(zbyva, 3) + " g");
            break;
        }

        Serial.print("Davkuji uhel: "); Serial.print(vhodnyUhel);
        Serial.print(", hmotnost: "); Serial.println(data[vhodnyUhel / 5].hmotnost);

        // Nastaveni serva a davkovani
        servo.write(offsetServo + vhodnyUhel);
        delay(1000); // Konstantni cas davkovani
        servo.write(offsetServo);

        delay(2000); // Cekani na stabilizaci vahy
        zpracujHX711();
        float novaHmotnost = currentWeight;
        hmotnostDosud += novaHmotnost;

        updateNextionText("currentW", String(hmotnostDosud, 2));
    }

    prefs.end();
    updateNextionText("status", String("Davkovani ") + namespaceName + " dokonceno");
    Serial.println("=== Davkovani dokonceno ===");
}