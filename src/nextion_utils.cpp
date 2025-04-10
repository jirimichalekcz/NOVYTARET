#include "nextion_utils.h"
#include "globals.h"

// Odeslání příkazu na Nextion displej
void sendNextionCommand(String command) {
  Serial2.print(command);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

// Aktualizace textového objektu na Nextion displeji
void updateNextionText(String objectName, String text) {
  Serial2.print(objectName);
  Serial2.print(".txt=\"");
  Serial2.print(text);
  Serial2.print("\"");
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

// Aktualizace numerického objektu (val)
void updateNextionValue(String objectName, int value) {
  Serial2.print(objectName);
  Serial2.print(".val=");
  Serial2.print(value);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}
