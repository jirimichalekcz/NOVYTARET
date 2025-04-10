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
    updateNextionText("status", String("Učení ") + namespaceName + " start");
    Serial.println(String("=== Učící režim ") + namespaceName + " zahájen ===");

    Preferences prefs;
    prefs.begin(namespaceName, false); // Otevření namespace
    prefs.clear(); // Vymazání všech předchozích dat
    prefs.end();

    prefs.begin(namespaceName, false); // Znovu otevření namespace

    const int krok = 5; // Krok 5 stupňů
    const int pocetUhlu = 19; // Pro úhly 0, 5, 10, ..., 90
    DosingData data[pocetUhlu]; // Pole pro uložení dat

    for (int i = 0; i < pocetUhlu; i++) {
        int relativeAngle = i * krok;
        int absoluteAngle = offsetServo + relativeAngle;

        updateNextionText("status", "Úhel " + String(absoluteAngle));
        Serial.println("---------------");
        Serial.print("Úhel "); Serial.print(absoluteAngle); Serial.println("°, nové měření");

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
        Serial.print("Rozdíl: "); Serial.println(rozdil, 3);

        data[i].uhel = relativeAngle;
        data[i].hmotnost = (rozdil < 0.4f) ? 0.0f : rozdil; // Pokud je rozdíl menší než 0.4 g, uložíme 0

        if (rozdil < 0.4f) {
            Serial.println("!!! Úhel " + String(absoluteAngle) + " přeskočen (malý rozdíl)");
            updateNextionText("status", "Úhel " + String(absoluteAngle) + " přeskočen");
        } else {
            Serial.print("✔ Uloženo pro úhel "); Serial.print(absoluteAngle);
            Serial.print("°: "); Serial.print(rozdil, 3); Serial.println(" g");
        }

        delay(1000);
    }

    // Uložení celého pole jako blob do NVS
    prefs.putBytes("dosingData", data, sizeof(data));
    Serial.println("✔ Všechna data byla uložena jako blob.");

    updateNextionText("status", String("Učení ") + namespaceName + " dokončeno");
    Serial.println(String("=== Učící režim ") + namespaceName + " dokončen ===");
    prefs.end();
}
