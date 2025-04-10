#pragma once

#include <Arduino.h>
#include <ESP32Servo.h> 
#include "globals.h"
void inicializujBzucak();
void inicializujSerial();
void inicializujHX711();


// 🧠 Pomocné funkce (utils) pro formatovani, výpisy apod.

/**
 * @brief Převede cas v milisekundach na formatovaný řetězec "HHH:MM"
 * 
 * @param timeMs cas v milisekundach
 * @return String ve formatu "HHH:MM"
 */
String formatTime(unsigned long timeMs);

/**
 * @brief Vypiše zvukový signal (pipnuti) na bzucaku
 * 
 * @param delka Délka pipnuti v ms
 */
void hrajZvuk(int delka);

/**
 * @brief Upozorňujici sekvence pipnuti (např. pro watchdog)
 */
void hrajVarovnyZvuk();

/**
 * @brief Zvukova sekvence při dokonceni davkovani v režimu MIX
 */
void hrajkonecMIXU();

/**
 * @brief Pomalu otevře servo z aktualniho úhlu do cilového
 * 
 * @param servo Servo objekt
 * @param aktualniUhel Výchozi úhel
 * @param cilovyUhel Cilový úhel
 * @param delayMs Pauza mezi kroky (pro pomalý pohyb)
 */
void pomaluOtevriServo(Servo &servo, int aktualniUhel, int cilovyUhel, int delayMs);

void tareScale();

