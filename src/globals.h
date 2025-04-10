#pragma once

// === Zakladni knihovny ===
#include <Arduino.h>
#include <Preferences.h>
#include <ESP32Servo.h>
#include <Q2HX711.h>
#include <vector>

// === Serial komunikace (Nextion displej) ===
const int NEXTION_RX = 15;
const int NEXTION_TX = 4;




extern int manualAngleA;
extern int manualAngleB;
extern bool manualModeActive;
extern unsigned long keyPressStartTime;
extern bool timeCounting;

extern int learningAngle;
extern float learningLastWeight;
extern bool learningIncreasing;
extern bool learningOffsetActive;

extern int learningAngleB;
extern float learningLastWeightB;
extern bool learningIncreasingB;
extern bool learningOffsetB_Active;

extern int currentPage;
extern long prumerRaw;

void uciciRezimServoA();
void uciciRezimServoB();


// === EEPROM / NVS ===
extern Preferences preferences;  // Objekt pro trvalé uloženi dat

// === Servo motor A a B ===
extern Servo servoA;
extern Servo servoB;

extern int pinServoA;            // GPIO pin pro servo A
extern int pinServoB;            // GPIO pin pro servo B
extern int offsetServoA;         // Zavřena pozice pro servo A (uci se)
extern int offsetServoB;         // Zavřena pozice pro servo B (uci se)

extern bool servoAOpened;
extern bool servoBOpened;

// === Bzucak ===
extern const int bzucak;         // Výstupni pin pro bzucak

// === Vahový senzor (HX711) ===
extern Q2HX711 hx711;            // Objekt pro praci s vahou
extern long offset;              // Offset kalibrace
extern float calibrationFactor;  // Kalibracni konstanta
extern float currentWeight;      // Aktualně změřena hmotnost

// === Davkovani a cilové hodnoty ===
extern float desiredWeight;      // Uživatelem zadana cilova hmotnost
extern float targetWeightA;      // Vypoctený cil pro složku A
extern float targetWeightB;      // Vypoctený cil pro složku B
extern float lastDesiredWeight;  // Posledni použita hodnota davky
extern float totalWeightA;       // Celkově nadavkovano složky A
extern float totalWeightB;       // Celkově nadavkovano složky B

// === Poměry složek ===
extern int slozkaA;              // Poměr složky A (např. 100)
extern int slozkaB;              // Poměr složky B (např. 40)

// === Stavový automat a režimy ===
enum State {
  WAITING_FOR_INPUT,   // ceka na vstup
  DOSING_A,            // Davkuje složku A
  DOSING_B,            // Davkuje složku B
  COMPLETED,           // Davkovani dokonceno
  LEARNING_OFFSET_A,   // Uci se offset pro servo A
  LEARNING_OFFSET_B    // Uci se offset pro servo B
};
extern State currentState;

enum DosingMode {
  NONE,                // Žadný režim
  COMPONENT_A,         // Pouze složka A
  COMPONENT_B,         // Pouze složka B
  MIX                  // Poměr A + B
};
extern DosingMode dosingMode;

// === Uživatelský vstup & vizualizace ===
extern String inputWeight;              // Text z Nextion klavesnice
extern std::vector<float> grafData;     // Data pro graf průběhu davkovani

