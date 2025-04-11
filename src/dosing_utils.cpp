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
    float casOtevreni = 1000; // Výchozí čas otevření serva (v ms)

    while (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        float nejblizsiHmotnost = 0.0f;
        int nejblizsiUhel = -1;

        // Vyhledani nejblizsiho vhodneho uhlu
        for (int i = 0; i < pocetUhlu; i++) {
            if (data[i].hmotnost > 0.01f && data[i].hmotnost <= zbyva) {
                if (data[i].hmotnost > nejblizsiHmotnost) {
                    nejblizsiHmotnost = data[i].hmotnost;
                    nejblizsiUhel = data[i].uhel;
                }
            }
        }

        if (nejblizsiUhel == -1) {
            Serial.println("Nelze davkovat presneji. Zbyva: " + String(zbyva, 3) + " g");
            break;
        }

        // Pouziti nejblizsiho uhlu
        Serial.print("Davkuji uhel: "); Serial.print(nejblizsiUhel);
        Serial.print(", ocekavana hmotnost: "); Serial.println(nejblizsiHmotnost);

        // Nastaveni serva a davkovani
        servo.write(offsetServo + nejblizsiUhel);
        delay(casOtevreni); // Dynamicky cas otevreni
        servo.write(offsetServo);

        // Cekani na stabilizaci vahy
        unsigned long startTime = millis();
        float namereno = 0.0f;
        while (millis() - startTime < 5000) { // Maximálně 5 sekund
            zpracujHX711();
            if (abs(currentWeight - namereno) < 0.1f) { // Stabilizace na ±0.1 g
                namereno = currentWeight;
                break;
            }
            delay(100);
        }

        Serial.print("Namerena hmotnost: ");
        Serial.println(namereno);

        // Kontrola odchylky
        if (abs(namereno - nejblizsiHmotnost) > 2.0f) { // Tolerance 2 g
            Serial.println("Varovani: Naměřená hmotnost se výrazně liší od očekávané!");
            Serial.print("Očekávaná: "); Serial.print(nejblizsiHmotnost);
            Serial.print(", Naměřená: "); Serial.println(namereno);
        }

        // Úprava času otevření na základě odchylky
        if (namereno > 0) {
            float pomer = nejblizsiHmotnost / namereno;
            casOtevreni = constrain(casOtevreni * pomer, 500, 1500); // Omezit čas mezi 500 ms a 1500 ms
            Serial.print("Upraveny cas otevreni: ");
            Serial.println(casOtevreni);
        }

        hmotnostDosud += namereno;
        updateNextionText("currentW", String(hmotnostDosud, 2));
    }

    prefs.end();

    if (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        Serial.println("Davkovani nedokonceno. Zbyva: " + String(zbyva, 3) + " g");
        updateNextionText("status", "Zbyva: " + String(zbyva, 3) + " g");
    } else {
        updateNextionText("status", String("Davkovani ") + namespaceName + " dokonceno");
        Serial.println("=== Davkovani dokonceno ===");
    }
}