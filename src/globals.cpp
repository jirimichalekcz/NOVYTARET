/**
 * @file globals.cpp
 * @brief Obsahuje globální proměnné a objekty používané v projektu.
 * 
 * Tento soubor definuje globální proměnné, jako jsou serva, váhový senzor,
 * bzučák, stavový automat a další, které jsou sdíleny mezi různými částmi projektu.
 * 
 * @author [jiri michalek]
 * @date [11042025]
 */

#include "globals.h"

// === Preferences (NVS) ===
/// Instance pro práci s NVS (Non-Volatile Storage).
Preferences preferences;

// === Servo motor A ===
/// Servo motor A.
Servo servoA;

/// Servo motor B.
Servo servoB;

/// Offset pro výchozí pozici serva A (nastavuje se v režimu učení).
int offsetServoA = 0;

/// Offset pro výchozí pozici serva B (nastavuje se v režimu učení).
int offsetServoB = 0;

/// GPIO pin pro servo A.
int pinServoA = 18;

/// GPIO pin pro servo B.
int pinServoB = 19;

// === Bzucak ===
/// GPIO pin, na kterém je připojen bzučák.
const int bzucak = 13;

// === Vahovy senzor (HX711) ===
/// Instance váhového senzoru HX711 (DATA pin, CLOCK pin).
Q2HX711 hx711(33, 32);

/// Offset pro váhový senzor (nastavuje se během kalibrace).
long offset = 0;

/// Kalibrační faktor pro váhový senzor.
float calibrationFactor = 3.55f;

/// Aktuální hmotnost naměřená váhovým senzorem.
float currentWeight = 0.0f;

// === Davkovani ===
/// Požadovaná celková hmotnost dávkování.
float desiredWeight = 0.0f;

/// Cílová hmotnost složky A.
float targetWeightA = 0.0f;

/// Cílová hmotnost složky B.
float targetWeightB = 0.0f;

/// Celková nadávkovaná hmotnost složky A.
float totalWeightA = 0.0f;

/// Celková nadávkovaná hmotnost složky B.
float totalWeightB = 0.0f;

/// Poslední požadovaná hmotnost (pro porovnání změn).
float lastDesiredWeight = 0.0f;

// === Poměry složek A / B ===
/// Poměr složky A (v procentech).
int slozkaA = 100;

/// Poměr složky B (v procentech).
int slozkaB = 40;

// === Stavovy automat a režimy ===
/// Aktuální stav stavového automatu.
State currentState = WAITING_FOR_INPUT;

/// Aktuální režim dávkování.
DosingMode dosingMode = NONE;

// === Uživatelsky vstup a grafy ===
/// Uživatelský vstup pro požadovanou hmotnost.
String inputWeight = "";

/// Data pro grafické zobrazení průběhu dávkování.
std::vector<float> grafData;

// === Stav serva (pro watchdog / bezpecnost) ===
/// Indikátor, zda je servo A otevřené.
bool servoAOpened = false;

/// Indikátor, zda je servo B otevřené.
bool servoBOpened = false;
