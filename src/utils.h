#pragma once

#include <Arduino.h>
#include <ESP32Servo.h> 
#include "globals.h"
void inicializujBzucak();
void inicializujSerial();
void inicializujHX711();


// 🧠 Pomocné funkce (utils) pro formatovani, vypisy apod.

/**
 * @brief Prevede cas v milisekundach na formatovany retězec "HHH:MM"
 * 
 * @param timeMs cas v milisekundach
 * @return String ve formatu "HHH:MM"
 */
String formatTime(unsigned long timeMs);

/**
 * @brief Vypiše zvukovy signal (pipnuti) na bzucaku
 * 
 * @param delka Délka pipnuti v ms
 */
void hrajZvuk(int delka);

/**
 * @brief Upozorňujici sekvence pipnuti (napr. pro watchdog)
 */
void hrajVarovnyZvuk();

/**
 * @brief Zvukova sekvence pri dokonceni davkovani v režimu MIX
 */
void hrajkonecMIXU();

/**
 * @brief Pomalu otevre servo z aktualniho Uhlu do cilového
 * 
 * @param servo Servo objekt
 * @param aktualniUhel Vychozi Uhel
 * @param cilovyUhel Cilovy Uhel
 * @param delayMs Pauza mezi kroky (pro pomaly pohyb)
 */
void pomaluOtevriServo(Servo &servo, int aktualniUhel, int cilovyUhel, int delayMs);

void tareScale();

