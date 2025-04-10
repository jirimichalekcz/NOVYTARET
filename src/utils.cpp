#include "utils.h"
#include "globals.h"  // pro pristup k bzucak, Servo apod.

String formatTime(unsigned long timeMs) {
    unsigned long totalMinutes = timeMs / 60000;
    unsigned long hours = totalMinutes / 60;
    unsigned long minutes = totalMinutes % 60;

    char buffer[10];
    sprintf(buffer, "%03lu:%02lu", hours, minutes);
    return String(buffer);
}

void hrajZvuk(int delka) {
    digitalWrite(bzucak, HIGH);
    delay(delka);
    digitalWrite(bzucak, LOW);
}

void hrajVarovnyZvuk() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(bzucak, HIGH);
        delay(300);
        digitalWrite(bzucak, LOW);
        delay(200);
    }
    delay(500);
    for (int i = 0; i < 2; i++) {
        digitalWrite(bzucak, HIGH);
        delay(500);
        digitalWrite(bzucak, LOW);
        delay(300);
    }
}

void hrajkonecMIXU() {
    for (int i = 0; i < 3; i++) {
        hrajZvuk(200);
        delay(200);
    }
}

void pomaluOtevriServo(Servo &servo, int aktualniUhel, int cilovyUhel, int delayMs) {
    for (int uhel = aktualniUhel; uhel <= cilovyUhel; uhel++) {
        servo.write(uhel);
        delay(delayMs);
    }
}


void inicializujBzucak() {
    pinMode(bzucak, OUTPUT);
    digitalWrite(bzucak, LOW);
  }
  
  void inicializujSerial() {
    Serial.begin(9600);

    Serial2.begin(9600, SERIAL_8N1, NEXTION_RX, NEXTION_TX);

  }
  
  void inicializujHX711() {
    delay(500);       // Nech cas na stabilizaci
    tareScale();      // Spusť tare hned na start
  }