#pragma once

// === HX711 vážení ===
void tareScale();
void zpracujHX711();
void vypisHmotnost(float hmotnost);
void kontrolujWatchdog();

struct DosingData {
    int uhel;          // Úhel serva
    float hmotnost;    // Hmotnost odpovídající úhlu
};
