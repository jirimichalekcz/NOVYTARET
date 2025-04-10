#pragma once

// === Základní knihovny ===
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
extern Preferences preferences;  // Objekt pro trvalé uložení dat

// === Servo motor A a B ===
extern Servo servoA;
extern Servo servoB;

extern int pinServoA;            // GPIO pin pro servo A
extern int pinServoB;            // GPIO pin pro servo B
extern int offsetServoA;         // Zavřená pozice pro servo A (učí se)
extern int offsetServoB;         // Zavřená pozice pro servo B (učí se)

extern bool servoAOpened;
extern bool servoBOpened;

// === Bzučák ===
extern const int bzucak;         // Výstupní pin pro bzučák

// === Váhový senzor (HX711) ===
extern Q2HX711 hx711;            // Objekt pro práci s váhou
extern long offset;              // Offset kalibrace
extern float calibrationFactor;  // Kalibrační konstanta
extern float currentWeight;      // Aktuálně změřená hmotnost

// === Dávkování a cílové hodnoty ===
extern float desiredWeight;      // Uživatelem zadaná cílová hmotnost
extern float targetWeightA;      // Vypočtený cíl pro složku A
extern float targetWeightB;      // Vypočtený cíl pro složku B
extern float lastDesiredWeight;  // Poslední použitá hodnota dávky
extern float totalWeightA;       // Celkově nadávkováno složky A
extern float totalWeightB;       // Celkově nadávkováno složky B

// === Poměry složek ===
extern int slozkaA;              // Poměr složky A (např. 100)
extern int slozkaB;              // Poměr složky B (např. 40)

// === Stavový automat a režimy ===
enum State {
  WAITING_FOR_INPUT,   // Čeká na vstup
  DOSING_A,            // Dávkuje složku A
  DOSING_B,            // Dávkuje složku B
  COMPLETED,           // Dávkování dokončeno
  LEARNING_OFFSET_A,   // Učí se offset pro servo A
  LEARNING_OFFSET_B    // Učí se offset pro servo B
};
extern State currentState;

enum DosingMode {
  NONE,                // Žádný režim
  COMPONENT_A,         // Pouze složka A
  COMPONENT_B,         // Pouze složka B
  MIX                  // Poměr A + B
};
extern DosingMode dosingMode;

// === Uživatelský vstup & vizualizace ===
extern String inputWeight;              // Text z Nextion klávesnice
extern std::vector<float> grafData;     // Data pro graf průběhu dávkování

