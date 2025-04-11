#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "globals.h"
#include "hx711_utils.h"
#include "utils.h"
#include "learning_utils.h"



// Dopředné deklarace, pokud nejsou v utils.h
void updateNextionText(String objectName, String text);
void hrajZvuk(int delka);


void uciciRezimServoA() {
    uciciRezimServo(servoA, offsetServoA, "flowmapA");
}

void uciciRezimServoB() {
    uciciRezimServo(servoB, offsetServoB, "flowmapB");
}

void uciciRezimServo(Servo &servo, int offsetServo, const char *namespaceName) {
    updateNextionText("status", String("Uceni ") + namespaceName + " start");
    Serial.println(String("=== Ucici režim ") + namespaceName + " zahajen ===");

    Preferences prefs;
    prefs.begin(namespaceName, false); // Otevřeni namespace
    prefs.clear(); // Vymazani všech předchozich dat
    prefs.end();

    prefs.begin(namespaceName, false); // Znovu otevřeni namespace

    const int krok = 5; // Krok 5 stupňů
    const int pocetUhlu = 19; // Pro úhly 0, 5, 10, ..., 90
    DosingData data[pocetUhlu]; // Pole pro uloženi dat

    for (int i = 0; i < pocetUhlu; i++) {
        int relativeAngle = i * krok;
        int absoluteAngle = offsetServo + relativeAngle;

        updateNextionText("status", "Úhel " + String(absoluteAngle));
        Serial.println("---------------");
        Serial.print("Úhel "); Serial.print(absoluteAngle); Serial.println("°, nové měřeni");

        tareScale();
        delay(500);
        zpracujHX711(); delay(200);
        float pred = currentWeight;

        servo.write(absoluteAngle);
        delay(1000); // Servo zůstane otevřené 1 sekundu
        servo.write(offsetServo);

        delay(500);
        zpracujHX711(); delay(200);
        float po = currentWeight;

        float rozdil = po - pred;
        Serial.print("Rozdil: "); Serial.println(rozdil, 3);

        data[i].uhel = relativeAngle;
        data[i].hmotnost = (rozdil < 0.4f) ? 0.0f : rozdil; // Pokud je rozdil menši než 0.4 g, uložime 0

        if (rozdil < 0.4f) {
            Serial.println("!!! Úhel " + String(absoluteAngle) + " přeskocen (malý rozdil)");
            updateNextionText("status", "Úhel " + String(absoluteAngle) + " přeskocen");
        } else {
            Serial.print("✔ Uloženo pro úhel "); Serial.print(absoluteAngle);
            Serial.print("°: "); Serial.print(rozdil, 3); Serial.println(" g");
        }

        delay(1000);
    }

    // Uloženi celého pole jako blob do NVS
    prefs.putBytes("dosingData", data, sizeof(data));
    Serial.println("✔ Všechna data byla uložena jako blob.");

    updateNextionText("status", String("Uceni ") + namespaceName + " dokonceno");
    Serial.println(String("=== Ucici režim ") + namespaceName + " dokoncen ===");
    prefs.end();
}



void vypisDosingData(const char *namespaceName) {
    Preferences prefs;
    prefs.begin(namespaceName, true); // Otevření namespace pro čtení

    const int pocetUhlu = 19; // Počet úhlů (0, 5, 10, ..., 90)
    DosingData data[pocetUhlu];

    // Načtení dat z NVS
    size_t velikost = prefs.getBytes("dosingData", data, sizeof(data));
    if (velikost != sizeof(data)) {
        Serial.println(String("Chyba: Data pro ") + namespaceName + " nebyla správně načtena.");
        prefs.end();
        return;
    }

    // Výpis dat na Serial monitor
    Serial.println(String("=== Data pro ") + namespaceName + " ===");
    for (int i = 0; i < pocetUhlu; i++) {
        Serial.print("Úhel: ");
        Serial.print(data[i].uhel);
        Serial.print("°, Hmotnost: ");
        Serial.print(data[i].hmotnost, 3);
        Serial.println(" g");
    }
    Serial.println(String("=== Konec dat pro ") + namespaceName + " ===");

    prefs.end(); // Ukončení práce s NVS
}