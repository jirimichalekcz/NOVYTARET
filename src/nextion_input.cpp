#include "nextion_input.h"
#include "nextion_utils.h"
#include "globals.h"
#include <Arduino.h>
#include "utils.h"
#include "hx711_utils.h"
#include "dosing_utils.h"
#include "learning_utils.h"

// Tady můžeš prípadně doplnit další #include, pokud chybí

void zpracujNextionData() {
    static unsigned long lastPressTime = 0;
    const unsigned long minPressInterval = 0;

    while (Serial2.available() > 0) {
        char c = (char)Serial2.read();

        if (c == (char)0xFF) continue;

        unsigned long now = millis();
        if (now - lastPressTime < minPressInterval) continue;
        lastPressTime = now;

        if ((c >= '0' && c <= '9') || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == '*' || c == '#') {
            hrajZvuk(100);
        }

        // === Režim manuálního ladění offsetů (e–h) ===
        if (c == 'e') { offsetServoA--; servoA.write(offsetServoA); preferences.putInt("offsetA", offsetServoA); updateNextionText("status", "Offset A +"); return; }
        if (c == 'f') { offsetServoA++; servoA.write(offsetServoA); preferences.putInt("offsetA", offsetServoA); updateNextionText("status", "Offset A -"); return; }
        if (c == 'g') { offsetServoB--; servoB.write(offsetServoB); preferences.putInt("offsetB", offsetServoB); updateNextionText("status", "Offset B +"); return; }
        if (c == 'h') { offsetServoB++; servoB.write(offsetServoB); preferences.putInt("offsetB", offsetServoB); updateNextionText("status", "Offset B -"); return; }

        // === Manuální ovládání serv (i–p) ===
        if (c == 'i') { manualAngleA += 5; manualAngleA = constrain(manualAngleA, 0, 180); servoA.write(manualAngleA); updateNextionText("status", "Servo A +5°"); manualModeActive = true; return; }
        if (c == 'j') { manualAngleA -= 5; manualAngleA = constrain(manualAngleA, 0, 180); servoA.write(manualAngleA); updateNextionText("status", "Servo A -5°"); manualModeActive = true; return; }
        if (c == 'k') { manualAngleB += 5; manualAngleB = constrain(manualAngleB, 0, 180); servoB.write(manualAngleB); updateNextionText("status", "Servo B +5°"); manualModeActive = true; return; }
        if (c == 'l') { manualAngleB -= 5; manualAngleB = constrain(manualAngleB, 0, 180); servoB.write(manualAngleB); updateNextionText("status", "Servo B -5°"); manualModeActive = true; return; }
        if (c == 'm') { servoA.write(90); manualAngleA = 90; manualModeActive = true; updateNextionText("status", "Servo A → 90°"); return; }
        if (c == 'n') { servoA.write(0); manualAngleA = 0; manualModeActive = true; updateNextionText("status", "Servo A → 0°"); return; }
        if (c == 'o') { servoB.write(90); manualAngleB = 90; manualModeActive = true; updateNextionText("status", "Servo B → 90°"); return; }
        if (c == 'p') { servoB.write(0); manualAngleB = 0; manualModeActive = true; updateNextionText("status", "Servo B → 0°"); return; }

        // === Príkazy pro davkovani, kalibraci, stránkování, mazání... ===
        if (inputWeight == "787878") {
            Serial.println("Kod 787878 detekovan. Vypisuji data z NVS pameti.");
            vypisDosingData("flowmapA");
            vypisDosingData("flowmapB");
            inputWeight = ""; // Reset vstupu
            return;
        }

        // === davkovani slozky A ===
        if (c == 'A' && currentState == WAITING_FOR_INPUT) {
            keyPressStartTime = millis();
            timeCounting = true;

            tareScale(); // Vynulování váhy pred zahájením davkovani
            updateNextionText("status", "Taring scale...");

            if (!inputWeight.isEmpty()) {
                desiredWeight = inputWeight.toFloat();
            } else {
                desiredWeight = preferences.getFloat("lastWeight", 0.0f);
            }
            targetWeightA = desiredWeight;
            targetWeightB = 0;
            inputWeight = "";

            preferences.putFloat("lastWeight", desiredWeight);
            Serial.print("Hmotnost pro slozku A ulozena: ");
            Serial.println(desiredWeight);

            dosingMode = COMPONENT_A;
            currentState = DOSING_A;
            Serial.println("Zahajuji davkovani pouze slozky A.");
            updateNextionText("status", "Dosing component A");
            return;
        }

        // === davkovani slozky B ===
        if (c == 'B' && currentState == WAITING_FOR_INPUT) {
            keyPressStartTime = millis();
            timeCounting = true;

            tareScale(); // Vynulování váhy pred zahájením davkovani
            updateNextionText("status", "Taring scale...");

            if (!inputWeight.isEmpty()) {
                desiredWeight = inputWeight.toFloat();
            } else {
                desiredWeight = preferences.getFloat("lastWeight", 0.0f);
            }
            targetWeightB = desiredWeight;
            targetWeightA = 0;
            inputWeight = "";

            preferences.putFloat("lastWeight", desiredWeight);
            Serial.print("Hmotnost pro slozku B ulozena: ");
            Serial.println(desiredWeight);

            dosingMode = COMPONENT_B;
            currentState = DOSING_B;
            Serial.println("Zahajuji davkovani pouze slozky B.");
            updateNextionText("status", "Dosing component B");
            return;
        }

        // === davkovani v režimu MIX ===
        if (c == 'D' && currentState == WAITING_FOR_INPUT) {
            keyPressStartTime = millis();
            timeCounting = true;

            tareScale(); // Vynulování váhy pred zahájením davkovani
            updateNextionText("status", "Taring scale...");

            updateNextionText("currentWA", ""); // Vymazat hodnotu
            updateNextionText("currentWB", ""); // Vymazat hodnotu

            if (!inputWeight.isEmpty()) {
                desiredWeight = inputWeight.toFloat();
            } else {
                desiredWeight = preferences.getFloat("lastWeight", 0.0f);
            }
            preferences.putFloat("lastWeight", desiredWeight);
            Serial.print("Hmotnost pro davkovani v režimu MIX ulozena: ");
            Serial.println(desiredWeight);

            vypocitejCile(); // Spočítá targetWeightA a targetWeightB
            inputWeight = "";
            dosingMode = MIX;
            currentState = DOSING_A;
            Serial.println("Zahajuji davkovani v poměru slozek A a B.");
            updateNextionText("status", "Dosing components A and B");
            return;
        }

        // === Prepínání stránek na Nextionu ===
        if (c == '#') {
            currentPage++;
            if (currentPage > 3) currentPage = 1;

            sendNextionCommand("page " + String(currentPage));
            Serial.println("Prepnuto na stránku " + String(currentPage));
            updateNextionText("status", "Page " + String(currentPage));
            return;
        }


        if (c == '*') {
          manualModeActive = false;
        
         currentState = WAITING_FOR_INPUT;
        
          // === Zavrení ventilů ===
          servoA.write(offsetServoA);
          servoAOpened = false;
         servoB.write(offsetServoB);
          servoBOpened = false;
          Serial.println("Ventily A a B byly okamzite uzavreny.");
        
          inputWeight = "";
          Serial.println("Zadavani hmotnosti bylo zruseno.");
          updateNextionText("status", "Input cancelled");
          updateNextionText("inputWeight", inputWeight);
        
        
          currentState = WAITING_FOR_INPUT;
          Serial.println("Program byl resetovan do vychoziho stavu.");
          updateNextionText("status", "Program reset");
        
          tareScale();
          Serial.println("tarovano na nulu.");
          updateNextionText("status", "Scale tared");
          return;
        }
        

        // === Číselny vstup (0–9) ===
        if (c >= '0' && c <= '9') {
            inputWeight += c;
            Serial.print("Zadavana hmotnost: ");
            Serial.println(inputWeight);
            updateNextionText("inputWeight", inputWeight);
        }
    }
}


