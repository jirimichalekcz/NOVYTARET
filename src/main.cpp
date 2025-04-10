


// Makro pro ladici vystupy
#define DEBUG_MODE // zakomentuj tento radek pro zakazani všech Serial.print

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)   // Nic se neprovadi
  #define DEBUG_PRINTLN(x) // Nic se neprovadi
#endif

#include "globals.h"
#include "utils.h"
#include "hx711_utils.h"
#include "dosing_utils.h"
#include "learning_utils.h"
#include "nextion_utils.h"
#include "nextion_input.h"









//#include <Keypad.h>
#include <Q2HX711.h>
#include <esp_system.h> // Knihovna pro funkci esp_restart
#include <Preferences.h> // Knihovna pro praci s NVS pameti
//Preferences preferences; // Objekt pro praci s NVS pameti
#include <ESP32Servo.h>




const float ZAVRIT_PRED_KONCEM_A = 0.0f;
const float ZAVRIT_PRED_KONCEM_B = 3.5f;  // Pridano pro slozku B

//int offsetServoA = 0;  // Posun zavrene pozice serva A
//int offsetServoB = 0;  // Posun zavrene pozice serva B

float korekceZavreniB = 0.0f;

long staryrozdil = 0; 


int manualAngleA = 0;
int manualAngleB = 0;
bool manualModeActive = false;



// === Offset learning variables ===
bool learningOffsetActive = false;
int learningAngle = 0;
float learningLastWeight = 0.0f;
bool learningIncreasing = false;
unsigned long lastLearningTime = 0;

bool learningOffsetB_Active = false;
int learningAngleB = 0;
float learningLastWeightB = 0.0f;
bool learningIncreasingB = false;
unsigned long lastLearningTimeB = 0;






// Maximalni uhly otevreni pro jednotlive slozky (nastavitelne)
const int MAX_UHEL_A = 115;  // napr. plne otevreni ventilu A
const int MAX_UHEL_B = 90;   // napr. plne otevreni ventilu B



// Promenne pro cteni z HX711
unsigned long lastReadTime = 0;
const unsigned long readInterval = 200; // Interval cteni v milisekundach
long jaRaw = 0;
long naRaw = 0;
//long prumerRaw = 0;
long rozdil = 0;

float lastWeightForWatchdog = 0.0f;
unsigned long lastWeightChangeTime = 0;
const unsigned long WATCHDOG_TIMEOUT = 10000; // 10 sekund


// --- Globalni promenne pro mikrofazi davkovani slozky B ---
unsigned long lastMicroStepTime = 0;
float lastMicroWeight = 0.0f;
unsigned long lastWeightIncreaseTime = 0;
int microAngle = 0;

bool mikrofazeAktivni = false;





// Pridani globalnich promennych pro cas sepnuti rele
unsigned long lastSaveTime = 0; // cas posledniho ulozeni do pameti
const unsigned long saveInterval = 60000; // 1 minuta (v ms)

unsigned long keyPressStartTime = 0; // Ulozeni casu stisku klavesy
bool timeCounting = false;           // Indikace, zda je aktivni casovac





// Globalni promenne pro sledovani stavu klaves
bool keyA_Pressed = false;
bool keyB_Pressed = false;





unsigned long lastDoseTime = 0;

// Pouziti Serial2 pro Nextion displej  
//#define RX2 15
//#define TX2 

// Promenne pro pravidelnou aktualizaci inputWeight
unsigned long lastInputWeightUpdate = 0;
const unsigned long inputWeightUpdateInterval = 500; // Interval aktualizace v ms




// Globalni promenna pro sledovani aktualni stranky na displeji
int currentPage = 0; // Vychozi stranka je 0



void updateNextionText(String objectName, String text);
void sendNextionCommand(String command);
void tareScale();
void vypocitejCile();
void zpracujHX711();
void vypisHmotnost(float hmotnost);
void aktualizujDavkovani();
//void davkujSlozku();
//void endPreferences();
void vypisDataServoB();




void setup() {


  Serial2.begin(9600, SERIAL_8N1, NEXTION_RX, NEXTION_TX);

  inicializujBzucak();
  inicializujSerial();
  inicializujHX711();

   //registrujStrankuGrafu();

  // === OTEVrENi NVS PAMeTI ===
  preferences.begin("davsyst", false); // "davsyst" = nazev prostoru pro ukladani

offsetServoA = preferences.getInt("offsetA", 0); // Vychozi hodnota 0
offsetServoB = preferences.getInt("offsetB", 0);






  // === Pripojeni serv ===
  servoA.attach(pinServoA);
  servoB.attach(pinServoB);
 
 servoA.write(offsetServoA); // zavrena pozice pro A
 servoB.write(offsetServoB); // zavrena pozice pro B


  // === Nacteni hodnot ze zaznamu ===
  slozkaA = preferences.getInt("slozkaA", 100);       // Pomer slozky A
  slozkaB = preferences.getInt("slozkaB", 40);        // Pomer slozky B
  totalWeightA = preferences.getFloat("totalWeightA", 0.0f);
  totalWeightB = preferences.getFloat("totalWeightB", 0.0f);
  calibrationFactor = preferences.getFloat("CALFACTOR", 3.55f); // Kalibracni faktor


  //lastDesiredWeight = preferences.getFloat("lastWeight", 0.0f); // Posledni davka
  

  if (preferences.isKey("lastWeight")) {
    lastDesiredWeight = preferences.getFloat("lastWeight");
  } else {
    Serial.println("Klic 'lastWeight' nebyl nalezen. Ukladam vychozi hodnotu.");
    lastDesiredWeight = 0.0f; // vychozi hodnota
    preferences.putFloat("lastWeight", lastDesiredWeight);
  }

  

  // === Vystupy do Serial monitoru ===
  Serial.print("Nactena hodnota CALFACTOR: ");
  Serial.println(calibrationFactor);
  Serial.print("Nactena posledni hmotnost: ");
  Serial.println(lastDesiredWeight);

  // === Zobrazeni na Nextion displeji ===
  updateNextionText("lastWeight", String(lastDesiredWeight, 1));
  updateNextionText("calibnfo", String(calibrationFactor));
  updateNextionText("totalWeightA", String(totalWeightA / 1000.0f, 1) + " kg");
  updateNextionText("totalWeightB", String(totalWeightB / 1000.0f, 1) + " kg");
  updateNextionText("slozkaA", String(slozkaA));
  updateNextionText("slozkaB", String(slozkaB));

  // === uvodni zvuk a stranka ===
  sendNextionCommand("page 0");
  hrajZvuk(100); delay(250);
  hrajZvuk(100); delay(250);
  hrajZvuk(100); delay(1000);
  sendNextionCommand("page 1");

  // === Inicializace vstupu ===
  updateNextionText("inputWeight", inputWeight);

}



void loop() {


  unsigned long start = millis();

  //sendNextionCommand("draw 319,0,0,239,63488");



  if (timeCounting) {  // Vypocet uplynuleho casu pro zobrazeni na displeji pouze informativni charakter
    unsigned long elapsedTime = millis() - keyPressStartTime;
    unsigned long minutes = (elapsedTime / 60000) % 60; // Minuty
    unsigned long seconds = (elapsedTime / 1000) % 60;  // Sekundy

    // Formatovani casu jako MM:SS
    char timeBuffer[6];
    sprintf(timeBuffer, "%02lu:%02lu", minutes, seconds);

    // Aktualizace casu na displeji v objektu "PRtime"
    updateNextionText("PRtime", String(timeBuffer));
}


  zpracujNextionData();  // NOVa funkce na cteni a parsovani prikazů z Nextionu
  zpracujHX711();
  aktualizujDavkovani();


//Celkove casy a nadavkovane mnozstvi ulozit do pameti
    if (millis() - lastSaveTime >= saveInterval) {
    lastSaveTime = millis();
    preferences.putFloat("totalWeightA", totalWeightA);
    preferences.putFloat("totalWeightB", totalWeightB);
    Serial.println("Celkove casy a nadavkovane mnozstvi ulozeny do pameti.");
}



  // Pravidelna aktualizace inputWeight na displeji
  if (millis() - lastInputWeightUpdate >= inputWeightUpdateInterval) {
    
    
    if (preferences.isKey("lastWeight")) {
      lastDesiredWeight = preferences.getFloat("lastWeight");
    } else {
      Serial.println("Klic 'lastWeight' nebyl nalezen. Ukladam vychozi hodnotu.");
      lastDesiredWeight = 0.0f; // vychozi hodnota
      preferences.putFloat("lastWeight", lastDesiredWeight);
    }
    

    lastInputWeightUpdate = millis();
    updateNextionText("inputWeight", inputWeight);


    updateNextionText("totalWeightA", String(totalWeightA / 1000, 1) + " kg");
    updateNextionText("totalWeightB", String(totalWeightB / 1000, 1) + " kg");




    //updateNextionText("pg", String(currentPage));    
    updateNextionText("calibnfo", String(calibrationFactor));
    updateNextionText("targetA", String(targetWeightA, 1));
    updateNextionText("targetB", String(targetWeightB, 1));
    updateNextionText("desiredWeight", String(lastDesiredWeight, 1));
    updateNextionText("slozkaA", String(slozkaA)); 
    updateNextionText("slozkaB", String(slozkaB));
  
  


int uhelA = servoA.read();
int uhelB = servoB.read();

updateNextionText("SVA", (uhelA < 0) ? "X" : String(uhelA));
updateNextionText("SVB", (uhelB < 0) ? "X" : String(uhelB));

updateNextionText("offsetA", String(offsetServoA));
updateNextionText("offsetB", String(offsetServoB));

updateNextionValue("z0", uhelA); // Gauge nebo progress bar pro servo A
updateNextionValue("z1", uhelB); // Gauge nebo progress bar pro servo B

}


  //kontrolujWatchdog();


  ///////////uceni A start

if (currentState == LEARNING_OFFSET_A && learningOffsetActive) {
  if (millis() - lastLearningTime > 250) {
    lastLearningTime = millis();
    float newWeight = currentWeight;
    if (!learningIncreasing) {
      learningAngle++;
      servoA.write(learningAngle);
      Serial.print("Zkoušim uhel: "); Serial.print(learningAngle);
      Serial.print(" - Hmotnost: "); Serial.println(newWeight);
      if (newWeight - learningLastWeight >= 0.2f) {
        learningIncreasing = true;
        Serial.println("Narůst zjišten - prechazim na zavirani");
      }
      learningLastWeight = newWeight;
    } else {
      learningAngle--;
      servoA.write(learningAngle);
      Serial.print("Zaviram - uhel: "); Serial.println(learningAngle);
      if (abs(newWeight - learningLastWeight) < 0.1f) {
        offsetServoA = learningAngle;

       preferences.putInt("offsetA", offsetServoA);

        servoA.write(offsetServoA);
        updateNextionText("status", "Offset A ulozen");
        Serial.print("Novy offset ulozen: "); Serial.println(offsetServoA);
        hrajZvuk(400);
        learningOffsetActive = false;
        currentState = WAITING_FOR_INPUT;
      }
      learningLastWeight = newWeight;
    }
  }
}



  //////////uceni A stop


  ////////uceni B start
if (currentState == LEARNING_OFFSET_B && learningOffsetB_Active) {
  if (millis() - lastLearningTimeB > 250) {
    lastLearningTimeB = millis();
    float newWeight = currentWeight;
    if (!learningIncreasingB) {
      learningAngleB++;
      servoB.write(learningAngleB);
      Serial.print("Zkoušim uhel B: "); Serial.print(learningAngleB);
      Serial.print(" - Hmotnost: "); Serial.println(newWeight);
      if (newWeight - learningLastWeightB >= 0.2f) {
        learningIncreasingB = true;
        Serial.println("Narůst zjišten - prechazim na zavirani (servo B)");
      }
      learningLastWeightB = newWeight;
    } else {
      learningAngleB--;
      servoB.write(learningAngleB);
      Serial.print("Zaviram B - uhel: "); Serial.println(learningAngleB);
      if (abs(newWeight - learningLastWeightB) < 0.1f) {
        offsetServoB = learningAngleB;
      
        offsetServoB = learningAngleB;
        preferences.putInt("offsetB", offsetServoB);

        servoB.write(offsetServoB);
        updateNextionText("status", "Offset B ulozen");
        Serial.print("Novy offset B ulozen: "); Serial.println(offsetServoB);
        hrajZvuk(400);
        learningOffsetB_Active = false;
        currentState = WAITING_FOR_INPUT;
      }
      learningLastWeightB = newWeight;
    }
  }
}


  ///////uceni B stop 



//  Serial.print("Loop trvala: ");
//  Serial.println(millis() - start);
  

} //end loop




void vypocitejCile() {
    // Nacteni pomeru slozek z pameti ESP32
    int ulozenaSlozkaA = preferences.getInt("slozkaA", 100); // Vychozi hodnota 100
    int ulozenaSlozkaB = preferences.getInt("slozkaB", 40);  // Vychozi hodnota 40

    // Vypocet celkoveho pomeru
    int celkovyPomer = ulozenaSlozkaA + ulozenaSlozkaB;

    // Vypocet cilove hmotnosti pro slozku A a B
    targetWeightA = desiredWeight * (ulozenaSlozkaA / (float)celkovyPomer);
    targetWeightB = desiredWeight * (ulozenaSlozkaB / (float)celkovyPomer);

    // Vypis do Serial monitoru
    Serial.print("Cilova hmotnost A: ");
    Serial.print(targetWeightA, 1);
    Serial.println(" g");
    Serial.print("Cilova hmotnost B: ");
    Serial.print(targetWeightB, 1);
    Serial.println(" g");

    // Aktualizace cilovych hmotnosti na displeji
    updateNextionText("targetA", String(targetWeightA, 1));
    updateNextionText("targetB", String(targetWeightB, 1));
}





// === uprava aktualizujDavkovani ===
void aktualizujDavkovani()   {
  if (dosingMode == COMPONENT_A && currentState == DOSING_A) {
    davkujSlozkuAUcenim(targetWeightA);
    currentState = WAITING_FOR_INPUT;
    dosingMode = NONE;
  } else if (dosingMode == COMPONENT_B && currentState == DOSING_B) {
    davkujSlozkuBUcenim(targetWeightB);
    currentState = WAITING_FOR_INPUT;
    dosingMode = NONE;
  } else if (dosingMode == MIX) {
    if (currentState == DOSING_A) {
      davkujSlozkuAUcenim(targetWeightA);
      Serial.println("Prechod na davkovani slozky B.");
      delay(500);
      hrajZvuk(500);
      delay(500);
      currentState = DOSING_B;
    } else if (currentState == DOSING_B) {
      davkujSlozkuBUcenim(targetWeightB);
      currentState = WAITING_FOR_INPUT;
      dosingMode = NONE;
      updateNextionText("status", "Dosing completed");
      hrajkonecMIXU();
    }
  }

  // Aktualizace displeje
  if (dosingMode == COMPONENT_A) {
    updateNextionText("currentWA", String(currentWeight, 1));
    updateNextionText("currentWB", "");
  } else if (dosingMode == COMPONENT_B) {
    updateNextionText("currentWA", "");
    updateNextionText("currentWB", String(currentWeight, 1));
  } else if (dosingMode == MIX) {
    if (currentState == DOSING_A) {
      updateNextionText("currentWA", String(currentWeight, 1));
      updateNextionText("currentWB", "");
    } else if (currentState == DOSING_B) {
      updateNextionText("currentWA", String(targetWeightA, 1));
      float slozkaB_dosud = currentWeight - targetWeightA;
      if (slozkaB_dosud < 0) slozkaB_dosud = 0;
      updateNextionText("currentWB", String(slozkaB_dosud, 1));
    }
  }

  int uhelA = servoA.read();
  int uhelB = servoB.read();
  updateNextionText("SVA", (uhelA < 0) ? "X" : String(uhelA));
  updateNextionText("SVB", (uhelB < 0) ? "X" : String(uhelB));
}






void aktivujServo(Servo &servo, int uhelAktivace, int uhelNavrat, unsigned long setrvaniMs) {
  servo.write(uhelAktivace);
  delay(setrvaniMs);
  servo.write(uhelNavrat);
}












//void endPreferences() {
//  preferences.end(); // Ukonceni prace s NVS (volitelne pri neaktivite)
//}





int vypocitejMaxUhelA(float cilovaHmotnost) {
  if (cilovaHmotnost >= 100.0f) return 90;
  else if (cilovaHmotnost >= 50.0f) return 60;
  else if (cilovaHmotnost >= 30.0f) return 45;
  else if (cilovaHmotnost >= 20.0f) return 35;
  else if (cilovaHmotnost >= 10.0f) return 25;
  else return 25;
}



int vypocitejMaxUhelB(float cilovaHmotnost) {
  if (cilovaHmotnost >= 100.0f) return 90;
  else if (cilovaHmotnost >= 50.0f) return 60;
  else if (cilovaHmotnost >= 30.0f) return 45;
  else if (cilovaHmotnost >= 20.0f) return 35;
  else if (cilovaHmotnost >= 10.0f) return 25;
  else return 25;
}





// === Ucici rezim davkovani pro servo B ===
Preferences flowPrefs; // novy objekt pro ukladani průtoků






