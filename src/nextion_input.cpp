#include "nextion_input.h"
#include "nextion_utils.h"
#include "globals.h"
#include <Arduino.h>
#include "utils.h"
#include "hx711_utils.h"
#include "dosing_utils.h"



// Tady můžeš případně doplnit další #include, pokud chybí

void zpracujNextionData() {
  static unsigned long lastPressTime = 0;
  const unsigned long minPressInterval = 0;

  while (Serial2.available() > 0) {
    char c = (char)Serial2.read();

    if (c == (char)0xFF) continue;

    unsigned long now = millis();
    if (now - lastPressTime < minPressInterval) continue;
    lastPressTime = now;

    if (
      (c >= '0' && c <= '9') ||
      c == 'A' || c == 'B' || c == 'C' || c == 'D' ||
      c == '*' || c == '#'
    ) {
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

    // === Příkazy pro dávkování, kalibraci, stránkování, mazání... ===
    // === Zde buď zkopíruj vše ze stávající funkce (větší část Nextion vstupu) ===
    // Doporučuju to tam přenést ve stejné podobě, případně později rozdělit do dalších funkcí
  
  




    // 1) Dávkování složky A
    if (c == 'A' && currentState == WAITING_FOR_INPUT) {

      keyPressStartTime = millis();
      timeCounting = true;


          tareScale(); // Vynulování váhy před zahájením dávkování
          updateNextionText("status", "Taring scale...");

      if (!inputWeight.isEmpty()) {
        desiredWeight = inputWeight.toFloat(); 
      } else {
        desiredWeight = preferences.getFloat("lastWeight", 0.0f); 
      }
      targetWeightA = desiredWeight;
      targetWeightB = 0;
      inputWeight = "";

      // Uložit poslední zadanou hmotnost
      preferences.putFloat("lastWeight", desiredWeight);
      Serial.print("Hmotnost pro složku A uložena: ");
      Serial.println(desiredWeight);

      dosingMode = COMPONENT_A;
      currentState = DOSING_A;
      Serial.println("Zahajuji dávkování pouze složky A.");
      updateNextionText("status", "Dosing component A");
      return;
    }

    // -------------------------------------------------------------------
    // 2) Dávkování složky B
    if (c == 'B' && currentState == WAITING_FOR_INPUT) {

      keyPressStartTime = millis();
      timeCounting = true;


          tareScale(); // Vynulování váhy před zahájením dávkování
          updateNextionText("status", "Taring scale...");


      if (!inputWeight.isEmpty()) {
        desiredWeight = inputWeight.toFloat(); 
      } else {
        desiredWeight = preferences.getFloat("lastWeight", 0.0f); 
      }
      targetWeightB = desiredWeight;
      targetWeightA = 0;
      inputWeight = "";

      // Uložit poslední zadanou hmotnost
      preferences.putFloat("lastWeight", desiredWeight);
      Serial.print("Hmotnost pro složku B uložena: ");
      Serial.println(desiredWeight);

      dosingMode = COMPONENT_B;
      currentState = DOSING_B;
      Serial.println("Zahajuji dávkování pouze složky B.");
      updateNextionText("status", "Dosing component B");
      return;
    }

    // -------------------------------------------------------------------
    // 3) Dávkování v režimu MIX (tlačítko 'D' v původním kódu)
    if (c == 'D' && currentState == WAITING_FOR_INPUT) {

      keyPressStartTime = millis();
      timeCounting = true;

      tareScale(); // Vynulování váhy před zahájením dávkování
      updateNextionText("status", "Taring scale...");

      updateNextionText("currentWA", ""); // Vymazat hodnotu
      updateNextionText("currentWB", ""); // Vymazat hodnotu
    

      if (!inputWeight.isEmpty()) {
        desiredWeight = inputWeight.toFloat();
      } else {
        desiredWeight = preferences.getFloat("lastWeight", 0.0f);
      }
      preferences.putFloat("lastWeight", desiredWeight);
      Serial.print("Hmotnost pro dávkování v režimu MIX uložena: ");
      Serial.println(desiredWeight);

      vypocitejCile();  // spočítá targetWeightA a targetWeightB
      inputWeight = "";
      dosingMode = MIX;
      currentState = DOSING_A; 
      Serial.println("Zahajuji dávkování v poměru složek A a B.");
      updateNextionText("status", "Dosing components A and B");
      return;
    }

    // -------------------------------------------------------------------
    // 4) Přepínání stránek na Nextionu (např. '#' přepíná page 1 / page 2)
if (c == '*') {
  manualModeActive = false;

 currentState = WAITING_FOR_INPUT;

  // === Zavření ventilů ===
  servoA.write(offsetServoA);
  servoAOpened = false;
 servoB.write(offsetServoB);
  servoBOpened = false;
  Serial.println("Ventily A a B byly okamžitě uzavřeny.");

  inputWeight = "";
  Serial.println("Zadávání hmotnosti bylo zrušeno.");
  updateNextionText("status", "Input cancelled");
  updateNextionText("inputWeight", inputWeight);


  currentState = WAITING_FOR_INPUT;
  Serial.println("Program byl resetován do výchozího stavu.");
  updateNextionText("status", "Program reset");

  tareScale();
  Serial.println("Váha byla vytárována na nulu.");
  updateNextionText("status", "Scale tared");
  return;
}



    // -------------------------------------------------------------------
    // 6) Uložení kalibrace / nastavení složek A,B (tlačítko 'C')
    if (c == 'C') {
      if (inputWeight.length() == 9 && inputWeight.substring(0, 3) == "123") {
        // Rozdělení vstupu na složky
        slozkaA = inputWeight.substring(3, 6).toInt();
        slozkaB = inputWeight.substring(6, 9).toInt();

        preferences.putInt("slozkaA", slozkaA);
        preferences.putInt("slozkaB", slozkaB);

        Serial.print("Složka A uložena: ");
        Serial.println(slozkaA);
        Serial.print("Složka B uložena: ");
        Serial.println(slozkaB);

        updateNextionText("status", "Components saved");
        inputWeight = "";
        hrajZvuk(500); // Signalizace OK
      } 
      else if (inputWeight == "112233") {
        int kalibracnicislo = prumerRaw - (offset / 100);
        float kalibracepro = kalibracnicislo / 100.00f;

        preferences.putFloat("CALFACTOR", kalibracepro);
        Serial.print("Hodnota kalibrace byla uložena: ");
        Serial.println(kalibracepro);

        updateNextionText("status", "CALFACTOR saved");
        inputWeight = "";

        // Zvuk a restart
        hrajZvuk(500); delay(500); hrajZvuk(500); delay(500); hrajZvuk(500); delay(500);
        esp_restart();
      }


     // === Offset A learning režim ===
  else if (inputWeight == "555555") {
  Serial.println("learning offsetu A");
  learningAngle = offsetServoA;
  learningLastWeight = currentWeight;
  learningIncreasing = false;
  learningOffsetActive = true;
  currentState = LEARNING_OFFSET_A;
  updateNextionText("status", "Learning offset A");
  inputWeight = "";
  return;
}


     // === Offset B learning režim ===
  else if (inputWeight == "666666") {
Serial.println("learning offsetu B");
  learningAngleB = offsetServoB;
  learningLastWeightB = currentWeight;
  learningIncreasingB = false;
  learningOffsetB_Active = true;
  currentState = LEARNING_OFFSET_B;
  updateNextionText("status", "Learning offset B");
  inputWeight = "";
  return;
}




      else if (inputWeight == "445566") {
        sendNextionCommand("page 0"); 
      }
      else if (inputWeight == "197511") {
        // Vynulování dat o načerpané hmotnosti
        totalWeightA = 0.0f;
        totalWeightB = 0.0f;
        preferences.putFloat("totalWeightA", totalWeightA);
        preferences.putFloat("totalWeightB", totalWeightB);

        Serial.println("Data o načerpané hmotnosti byla vynulována.");
        updateNextionText("totalWeightA", "0.0 kg");
        updateNextionText("totalWeightB", "0.0 kg");
        updateNextionText("status", "Weights reset");

        hrajZvuk(500); delay(200); hrajZvuk(500);
        inputWeight = "";
      }

      else if (inputWeight == "757575") {
  Serial.println("Učící režim dávkování složky B spuštěn.");
  inputWeight = "";
  uciciRezimServoB();
  return;
}

      else if (inputWeight == "747474") {
  Serial.println("Učící režim dávkování složky A spuštěn.");
  inputWeight = "";
  uciciRezimServoA();
  return;
}



else if (inputWeight == "888888") {
  Serial.println("DELETE NVS B DATA");
  preferences.begin("flowmapB", false); // otevři pro zápis

  for (int angle = 1; angle <= 90; angle += 1) {
    String key = "B" + String(angle);
    preferences.remove((key + "g").c_str());
    preferences.remove((key + "t").c_str());
  }
  preferences.end(); // přidat zde
  updateNextionText("status", "Data B DELETED");
  hrajZvuk(200);
  delay(300);
  hrajZvuk(200);

  inputWeight = "";
  return;
}



else if (inputWeight == "505509") {
  Serial.println("!!! GLOBÁLNÍ MAZÁNÍ VŠECH ULOŽENÝCH DAT V NVS !!!");

  // Vymaž složku davsyst
  Preferences sysPrefs;
  sysPrefs.begin("davsyst", false);
  sysPrefs.clear();
  sysPrefs.end();

  // Vymaž flowmapA
  Preferences flowA;
  flowA.begin("flowmapA", false);
  flowA.clear();
  flowA.end();

  // Vymaž flowmapB
  Preferences flowB;
  flowB.begin("flowmapB", false);
  flowB.clear();
  flowB.end();

  updateNextionText("status", "🧹 NVS paměť vymazána");
  hrajZvuk(400); delay(200); hrajZvuk(400); delay(200); hrajZvuk(400);

  Serial.println("✅ Hotovo. Restart ESP32...");
  delay(1000);
  esp_restart(); // Restart pro nový začátek

  return;
}





else if (inputWeight == "777777") {
  Serial.println("DELETE NVS A DATA");
Preferences flowPrefsA;
flowPrefsA.begin("flowmapA", false);
for (int angle = 1; angle <= 90; angle += 1) {
  String key = "A" + String(angle);
  flowPrefsA.remove((key + "g").c_str());
  flowPrefsA.remove((key + "t").c_str());
}
flowPrefsA.end();


  updateNextionText("status", "Data A DELETED");
  hrajZvuk(200);
  delay(300);
  hrajZvuk(200);

  inputWeight = "";
  return;
}




      else {
        Serial.println("Klávesa C stisknuta, ale podmínka pro uložení nebyla splněna.");
        updateNextionText("status", "Invalid input for save");
      }
      return;
    }

    // -------------------------------------------------------------------
    // 7) Pokud je to číselný vstup (0–9), přidat do inputWeight:
    if (c >= '0' && c <= '9') {
      inputWeight += c; 
      Serial.print("Zadávaná hmotnost: ");
      Serial.println(inputWeight);
      updateNextionText("inputWeight", inputWeight);
    }



// -------------------------------------------------------------------
// 8) Přepnutí mezi stránkami (např. # pro přepnutí page 1 <-> page 2)
if (c == '#') {
  currentPage++;

  if (currentPage > 3) currentPage = 1;

  sendNextionCommand("page " + String(currentPage));
  Serial.println("Přepnuto na stránku " + String(currentPage));
  updateNextionText("status", "Page " + String(currentPage));

  return;
}


  }

}


