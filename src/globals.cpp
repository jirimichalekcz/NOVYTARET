#include "globals.h"

// === Preferences (NVS) ===
Preferences preferences;

// === Servo motor A a B ===
Servo servoA;
Servo servoB;
int offsetServoA = 0;     // Výchozi offset – uci se v režimu uceni
int offsetServoB = 0;
int pinServoA = 18;       // GPIO pin pro servo A
int pinServoB = 19;       // GPIO pin pro servo B

// === Bzucak ===
const int bzucak = 13;

// === Vahový senzor (HX711) ===
Q2HX711 hx711(33, 32);     // DATA pin, CLOCK pin
long offset = 0;
float calibrationFactor = 3.55f;
float currentWeight = 0.0f;

// === Davkovani ===
float desiredWeight = 0.0f;
float targetWeightA = 0.0f;
float targetWeightB = 0.0f;
float totalWeightA = 0.0f;
float totalWeightB = 0.0f;
float lastDesiredWeight = 0.0f;

// === Poměry složek A / B ===
int slozkaA = 100;
int slozkaB = 40;

// === Stavový automat a režimy ===
State currentState = WAITING_FOR_INPUT;
DosingMode dosingMode = NONE;

// === Uživatelský vstup a grafy ===
String inputWeight = "";
std::vector<float> grafData;

// === Stav serva (pro watchdog / bezpecnost) ===
bool servoAOpened = false;
bool servoBOpened = false;
