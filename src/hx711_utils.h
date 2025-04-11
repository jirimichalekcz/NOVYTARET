#pragma once

// === HX711 važeni ===
void tareScale();
void zpracujHX711();
void vypisHmotnost(float hmotnost);
void kontrolujWatchdog();

struct DosingData {
    int uhel;          // Uhel serva
    float hmotnost;    // Hmotnost odpovidajici Uhlu
};
