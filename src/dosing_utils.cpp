#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <Q2HX711.h>
#include <ESP32Servo.h>
#include <cfloat> // Pro FLT_MAX
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
    float casOtevreni = 1000; // Vychodsi cas otevreni serva (v ms)

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
        float posledniHmotnost = currentWeight; // Ulozi aktualni hmotnost jako vychozi
        float namereno = 0.0f;

        while (millis() - startTime < 7000) { // Maximalne 7 sekund
            zpracujHX711(); // Aktualizace currentWeight
            Serial.print("Aktualni hmotnost: ");
            Serial.println(currentWeight);

            // Kontrola stabilizace
            if (abs(currentWeight - posledniHmotnost) < 0.2f) { // Stabilizace na ±0.2 g
                namereno = currentWeight; // Stabilizovana hodnota
                break;
            }

            posledniHmotnost = currentWeight; // Aktualizace posledni hodnoty
            delay(500); // Zvysena prodleva mezi merenimi na 500 ms
        }

        Serial.print("Namerena hmotnost: ");
        Serial.println(namereno);

        // Kontrola odchylky
        if (namereno > nejblizsiHmotnost * 1.5f) { // Pokud je namerena hmotnost o 50 % vyssi
            Serial.println("Chyba: Namerena hmotnost je prilis vysoka. Kontrola davkovani.");
            namereno = nejblizsiHmotnost; // Omezit na ocekavanou hmotnost
        }

        // Aktualizace celkove nadavkovane hmotnosti
        hmotnostDosud += namereno;
        if (hmotnostDosud >= cilovaHmotnost) {
            Serial.println("Cilova hmotnost dosazena nebo prekrocena.");
            break;
        }

        Serial.print("Celkem nadavkovano: ");
        Serial.println(hmotnostDosud);

        // Vypocet zbyvajici hmotnosti
        zbyva = cilovaHmotnost - hmotnostDosud;
        Serial.print("Zbyva nadavkovat: ");
        Serial.println(zbyva);

        // Uprava casu otevreni na zaklade odchylky
        if (namereno > 0) {
            float pomer = nejblizsiHmotnost / namereno;
            float uprava = constrain(pomer, 0.8, 1.2); // Omezit upravu na ±20 %
            casOtevreni = constrain(casOtevreni * uprava, 500, 1500); // Omezit cas mezi 500 ms a 1500 ms
            Serial.print("Upraveny cas otevreni: ");
            Serial.println(casOtevreni);
        }
    }

    prefs.end();

    if (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        Serial.println("Davkovani nedokonceno. Zbyva: " + String(zbyva, 3) + " g");

        // Pokus o dodavkovani
        if (zbyva > 0.01f) { // Pokud zbyva nadavkovat vice nez 0.01 g
            Serial.println("Pokus o jemne dodavkovani zbyvajici hmotnosti...");

            // Najit nejmensi uhel, ktery muze byt pouzit
            int nejmensiUhel = -1;
            float nejmensiHmotnost = FLT_MAX;

            for (int i = 0; i < pocetUhlu; i++) {
                if (data[i].hmotnost > 0.01f && data[i].hmotnost < nejmensiHmotnost) {
                    nejmensiHmotnost = data[i].hmotnost;
                    nejmensiUhel = data[i].uhel;
                }
            }

            if (nejmensiUhel != -1) {
                Serial.print("Pouzivam nejmensi uhel: ");
                Serial.print(nejmensiUhel);
                Serial.print(", ocekavana hmotnost: ");
                Serial.println(nejmensiHmotnost);

                // Jemne dodavkovani
                while (zbyva > 0.01f) {
                    float casJemnehoOtevreni = constrain(casOtevreni * (zbyva / nejmensiHmotnost), 100, 500); // Dynamicky cas otevreni
                    Serial.print("Jemne dodavkovani s casem otevreni: ");
                    Serial.println(casJemnehoOtevreni);

                    // Nastaveni serva a pokus o dodavkovani
                    servo.write(offsetServo + nejmensiUhel);
                    delay(casJemnehoOtevreni); // Pouzit kratky cas otevreni
                    servo.write(offsetServo);

                    // Cekani na stabilizaci vahy
                    unsigned long startTime = millis();
                    float posledniHmotnost = currentWeight;
                    float namereno = 0.0f;

                    while (millis() - startTime < 7000) { // Maximalne 7 sekund
                        zpracujHX711();
                        if (abs(currentWeight - posledniHmotnost) < 0.2f) {
                            namereno = currentWeight;
                            break;
                        }
                        posledniHmotnost = currentWeight;
                        delay(500);
                    }

                    Serial.print("Namerena hmotnost po jemnem dodavkovani: ");
                    Serial.println(namereno);

                    // Aktualizace celkove nadavkovane hmotnosti
                    hmotnostDosud += namereno;
                    zbyva = cilovaHmotnost - hmotnostDosud;

                    Serial.print("Celkem nadavkovano: ");
                    Serial.println(hmotnostDosud);
                    Serial.print("Zbyva nadavkovat: ");
                    Serial.println(zbyva);

                    // Pokud zbyvajici hmotnost je prilis mala, ukoncit
                    if (zbyva <= 0.01f) {
                        break;
                    }
                }
            } else {
                Serial.println("Nelze najit vhodny uhel pro jemne dodavkovani.");
            }
        }

        // Pokud stale zbyva hmotnost, informovat uzivatele
        if (zbyva > 0.01f) {
            updateNextionText("status", "Zbyva: " + String(zbyva, 3) + " g");
        } else {
            updateNextionText("status", String("Davkovani ") + namespaceName + " dokonceno");
            Serial.println("=== Davkovani dokonceno ===");
        }
    } else {
        updateNextionText("status", String("Davkovani ") + namespaceName + " dokonceno");
        Serial.println("=== Davkovani dokonceno ===");
    }
}