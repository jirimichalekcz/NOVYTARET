#pragma once

#include <Arduino.h>
#include <ESP32Servo.h> 
#include "globals.h"
void inicializujBzucak();
void inicializujSerial();
void inicializujHX711();


// 🧠 Pomocné funkce (utils) pro formátování, výpisy apod.

/**
 * @brief Převede čas v milisekundách na formátovaný řetězec "HHH:MM"
 * 
 * @param timeMs Čas v milisekundách
 * @return String ve formátu "HHH:MM"
 */
String formatTime(unsigned long timeMs);

/**
 * @brief Vypíše zvukový signál (pípnutí) na bzučáku
 * 
 * @param delka Délka pípnutí v ms
 */
void hrajZvuk(int delka);

/**
 * @brief Upozorňující sekvence pípnutí (např. pro watchdog)
 */
void hrajVarovnyZvuk();

/**
 * @brief Zvuková sekvence při dokončení dávkování v režimu MIX
 */
void hrajkonecMIXU();

/**
 * @brief Pomalu otevře servo z aktuálního úhlu do cílového
 * 
 * @param servo Servo objekt
 * @param aktualniUhel Výchozí úhel
 * @param cilovyUhel Cílový úhel
 * @param delayMs Pauza mezi kroky (pro pomalý pohyb)
 */
void pomaluOtevriServo(Servo &servo, int aktualniUhel, int cilovyUhel, int delayMs);

void tareScale();

