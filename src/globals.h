/**
 * @file globals.h
 * @brief Deklarace globálních proměnných, konstant a funkcí pro projekt.
 * 
 * Tento soubor obsahuje deklarace globálních proměnných, konstant, funkcí a výčtových typů,
 * které jsou sdíleny mezi různými částmi projektu. Obsahuje také definice stavového automatu
 * a režimů dávkování.
 * 
 * @author [jiri michalek]
 * @date [11042025]
 */

#pragma once

// === Zakladni knihovny ===
#include <Arduino.h>
#include <Preferences.h>
#include <ESP32Servo.h>
#include <Q2HX711.h>
#include <vector>

// === Serial komunikace (Nextion displej) ===

/// GPIO pin pro RX (příjem dat) displeje Nextion.
const int NEXTION_RX = 15;

/// GPIO pin pro TX (odesílání dat) displeje Nextion.
const int NEXTION_TX = 4;

/// Úhel serva A v manuálním režimu.
extern int manualAngleA;

/// Úhel serva B v manuálním režimu.
extern int manualAngleB;

/// Indikátor, zda je aktivní manuální režim.
extern bool manualModeActive;

/// Čas zahájení stisku tlačítka v manuálním režimu.
extern unsigned long keyPressStartTime;

/// Indikátor, zda se počítá čas stisku tlačítka.
extern bool timeCounting;

/// Úhel serva A během učení offsetu.
extern int learningAngle;

/// Poslední naměřená hmotnost během učení offsetu pro servo A.
extern float learningLastWeight;

/// Indikátor, zda se hmotnost během učení offsetu zvyšuje (servo A).
extern bool learningIncreasing;

/// Indikátor, zda je aktivní učení offsetu pro servo A.
extern bool learningOffsetActive;

/// Úhel serva B během učení offsetu.
extern int learningAngleB;

/// Poslední naměřená hmotnost během učení offsetu pro servo B.
extern float learningLastWeightB;

/// Indikátor, zda se hmotnost během učení offsetu zvyšuje (servo B).
extern bool learningIncreasingB;

/// Indikátor, zda je aktivní učení offsetu pro servo B.
extern bool learningOffsetB_Active;

extern int currentPage;
extern long prumerRaw;

void uciciRezimServoA();
void uciciRezimServoB();

// === EEPROM / NVS ===
/// Objekt pro práci s NVS (Non-Volatile Storage).
extern Preferences preferences;  // Objekt pro trvalé uloženi dat

// === Servo motor A ===
/// Servo motor A.
extern Servo servoA;

/// GPIO pin pro servo A.
extern int pinServoA;

/// Zavřená pozice pro servo A (nastavuje se během učení).
extern int offsetServoA;

/// Indikátor, zda je servo A otevřené.
extern bool servoAOpened;

// === Servo motor B ===
/// Servo motor B.
extern Servo servoB;

/// GPIO pin pro servo B.
extern int pinServoB;

/// Zavřená pozice pro servo B (nastavuje se během učení).
extern int offsetServoB;

/// Indikátor, zda je servo B otevřené.
extern bool servoBOpened;

// === Bzucak ===
/// GPIO pin, na kterém je připojen bzučák.
extern const int bzucak;

// === Vahovy senzor (HX711) ===

/// Objekt pro práci s váhovým senzorem HX711.
extern Q2HX711 hx711;

/// Offset kalibrace váhového senzoru.
extern long offset;

/// Kalibrační konstanta váhového senzoru.
extern float calibrationFactor;

/// Aktuálně naměřená hmotnost.
extern float currentWeight;

// === Davkovani a cilové hodnoty ===
/// Uživatelem zadaná cílová hmotnost.
extern float desiredWeight;

/// Vypočtená cílová hmotnost složky A.
extern float targetWeightA;

/// Vypočtená cílová hmotnost složky B.
extern float targetWeightB;

/// Poslední použitá hodnota dávky.
extern float lastDesiredWeight;

/// Celkově nadávkovaná hmotnost složky A.
extern float totalWeightA;

/// Celkově nadávkovaná hmotnost složky B.
extern float totalWeightB;

// === Poměry složek ===
/// Poměr složky A (např. 100 %).
extern int slozkaA;

/// Poměr složky B (např. 40 %).
extern int slozkaB;

// === Stavovy automat a režimy ===

/**
 * @brief Stavový automat pro řízení procesu dávkování.
 */
enum State {
  WAITING_FOR_INPUT,   ///< Čeká na vstup od uživatele.
  DOSING_A,            ///< Dávkuje složku A.
  DOSING_B,            ///< Dávkuje složku B.
  COMPLETED,           ///< Dávkování dokončeno.
  LEARNING_OFFSET_A,   ///< Učí se offset pro servo A.
  LEARNING_OFFSET_B    ///< Učí se offset pro servo B.
};
extern State currentState;

/**
 * @brief Režimy dávkování.
 */
enum DosingMode {
  NONE,                ///< Žádný režim.
  COMPONENT_A,         ///< Pouze složka A.
  COMPONENT_B,         ///< Pouze složka B.
  MIX                  ///< Poměr složek A + B.
};
extern DosingMode dosingMode;

// === Uživatelsky vstup & vizualizace ===
/// Text zadaný uživatelem na klávesnici Nextion.
extern String inputWeight;

/// Data pro grafické zobrazení průběhu dávkování.
extern std::vector<float> grafData;

