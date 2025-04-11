/**
 * @file dosing_utils.h
 * @brief Deklarace funkcí pro dávkování složek a výpočet cílů.
 * 
 * Tento soubor obsahuje deklarace funkcí pro dávkování složek A a B,
 * výpočet cílových hmotností a hlavní dávkovací funkci.
 * 
 * @author [jiri michalek]
 * @date [11042025]
 */

#pragma once

/**
 * @brief Dávkuje složku A pomocí předem naučených kalibračních dat.
 * 
 * Tato funkce dávkuje složku A na základě cílové hmotnosti a kalibračních dat
 * uložených v prostoru NVS `flowmapA`.
 * 
 * @param cilovaHmotnost Cílová hmotnost složky A (v gramech).
 */
void davkujSlozkuAUcenim(float cilovaHmotnost);

/**
 * @brief Dávkuje složku B pomocí předem naučených kalibračních dat.
 * 
 * Tato funkce dávkuje složku B na základě cílové hmotnosti a kalibračních dat
 * uložených v prostoru NVS `flowmapB`.
 * 
 * @param cilovaHmotnost Cílová hmotnost složky B (v gramech).
 */
void davkujSlozkuBUcenim(float cilovaHmotnost);

/**
 * @brief Vypočítá cílové hmotnosti složek A a B.
 * 
 * Tato funkce načte poměry složek
 */
void vypocitejCile();

void davkujSlozku(float cilovaHmotnost, Servo &servo, int offsetServo, const char *namespaceName);