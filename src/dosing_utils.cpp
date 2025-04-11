/**
 * @file dosing_utils.cpp
 * @brief Obsahuje funkce pro dávkování složek a jemné dodávkování.
 * 
 * Tento soubor implementuje hlavní logiku dávkování složek pomocí serv a váhy HX711.
 * Obsahuje funkce pro dávkování složek A a B, jemné dodávkování a práci s kalibračními daty.
 * 
 * @author [Jiri Michalek]
 * @date [11042025]
 */

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

/**
 * @brief Hlavní funkce pro dávkování složky.
 * 
 * Tato funkce dávkuje složku na základě cílové hmotnosti a kalibračních dat uložených v NVS.
 * Používá servo pro dávkování a váhu HX711 pro měření aktuální hmotnosti. Pokud je potřeba,
 * provádí jemné dodávkování pro dosažení přesné cílové hmotnosti.
 * 
 * @param cilovaHmotnost Cílová hmotnost složky (v gramech).
 * @param servo Odkaz na servo objekt, které ovládá dávkování.
 * @param offsetServo Offset pro výchozí pozici serva.
 * @param namespaceName Název prostoru v NVS, kde jsou uložena kalibrační data.
 */
void davkujSlozku(float cilovaHmotnost, Servo &servo, int offsetServo, const char *namespaceName);

/**
 * @brief Dávkuje složku A pomocí předem naučených kalibračních dat.
 * 
 * Tato funkce volá hlavní dávkovací funkci `davkujSlozku` s parametry specifickými
 * pro složku A. Používá servo A a kalibrační data uložená v prostoru NVS `flowmapA`.
 * 
 * @param cilovaHmotnost Cílová hmotnost složky A (v gramech).
 */
void davkujSlozkuAUcenim(float cilovaHmotnost) {
    davkujSlozku(cilovaHmotnost, servoA, offsetServoA, "flowmapA");
}

/**
 * @brief Dávkuje složku B pomocí předem naučených kalibračních dat.
 * 
 * Tato funkce volá hlavní dávkovací funkci `davkujSlozku` s parametry specifickými
 * pro složku B. Používá servo B a kalibrační data uložená v prostoru NVS `flowmapB`.
 * 
 * @param cilovaHmotnost Cílová hmotnost složky B (v gramech).
 */
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
    /**
     * @brief Načítá kalibrační data z NVS.
     * 
     * Data obsahují úhly serva a odpovídající hmotnosti, které byly naučeny během kalibrace.
     * Pokud se data nepodaří načíst, funkce ukončí dávkování.
     */

    float hmotnostDosud = 0.0f;
    float casOtevreni = 1000; // Vychodsi cas otevreni serva (v ms)

    while (hmotnostDosud < cilovaHmotnost) {
        float zbyva = cilovaHmotnost - hmotnostDosud;
        float nejblizsiHmotnost = 0.0f;
        int nejblizsiUhel = -1;

        // Vyhledani nejblizsiho vhodneho uhlu
        /**
         * @brief Vyhledává nejbližší úhel serva pro dávkování.
         * 
         * Prochází kalibrační data a hledá úhel, který odpovídá hmotnosti
         * nejbližší k požadované zbývající hmotnosti.
         */
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
        if (namereno > 0) {
            hmotnostDosud += namereno;
            Serial.print("Celkem nadavkovano: ");
            Serial.println(hmotnostDosud);
        } else {
            Serial.println("Chyba: Namereno je nulove nebo zaporne, hmotnost nebude aktualizovana.");
        }

        // Vypocet zbyvajici hmotnosti
        zbyva = cilovaHmotnost - hmotnostDosud;
        Serial.print("Zbyva nadavkovat: ");
        Serial.println(zbyva);

        // Ladici vypisy
        Serial.print("DEBUG: Namereno: ");
        Serial.println(namereno);
        Serial.print("DEBUG: Hmotnost dosud: ");
        Serial.println(hmotnostDosud);
        Serial.print("DEBUG: Zbyva: ");
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

                /**
                 * @brief Jemné dodávkování pro dosažení přesné cílové hmotnosti.
                 * 
                 * Používá nejmenší kalibrovaný úhel serva a krátký čas otevření
                 * pro dodání malého množství materiálu. Kontroluje stabilizaci váhy
                 * a aktualizuje zbývající hmotnost.
                 */
                while (zbyva > 0.01f) {
                    float casJemnehoOtevreni = constrain(casOtevreni * (zbyva / nejmensiHmotnost), 300, 500); // Zvysena minimalni hodnota na 300 ms
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
                        if (abs(currentWeight - posledniHmotnost) < 0.2f) { // Stabilizace na ±0.2 g
                            namereno = currentWeight - hmotnostDosud; // Rozdil od dosud nadavkovane hmotnosti
                            if (namereno > 0) { // Kontrola, zda je namereno kladne
                                break;
                            }
                        }
                        posledniHmotnost = currentWeight;
                        delay(500);
                    }

                    if (namereno > 0) {
                        hmotnostDosud += namereno;
                        zbyva = cilovaHmotnost - hmotnostDosud;

                        Serial.print("Namerena hmotnost po jemnem dodavkovani: ");
                        Serial.println(namereno);
                        Serial.print("Celkem nadavkovano: ");
                        Serial.println(hmotnostDosud);
                        Serial.print("Zbyva nadavkovat: ");
                        Serial.println(zbyva);
                    } else {
                        Serial.println("Chyba: Namereno je nulove nebo zaporne, hmotnost nebude aktualizovana.");
                        break;
                    }

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

            // 3x kratky zvuk
            for (int i = 0; i < 2; i++) {
                digitalWrite(bzucak, HIGH); // Zapnout bzučák
                delay(200);                // Krátký zvuk (200 ms)
                digitalWrite(bzucak, LOW); // Vypnout bzučák
                delay(200);                // Pauza mezi zvuky (200 ms)
            }
            /**
             * @brief Přehrává 3 krátké zvuky pomocí bzučáku.
             * 
             * Signalizuje dokončení dávkování.
             */
        }
    } else {
        updateNextionText("status", String("Davkovani ") + namespaceName + " dokonceno");
        Serial.println("=== Davkovani dokonceno ===");

        // 3x kratky zvuk
        for (int i = 0; i < 2; i++) {
            digitalWrite(bzucak, HIGH); // Zapnout bzučák
            delay(200);                // Krátký zvuk (200 ms)
            digitalWrite(bzucak, LOW); // Vypnout bzučák
            delay(200);                // Pauza mezi zvuky (200 ms)
        }
        /**
         * @brief Přehrává 3 krátké zvuky pomocí bzučáku.
         * 
         * Signalizuje dokončení dávkování.
         */
    }
}