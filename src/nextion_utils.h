#pragma once
#include <Arduino.h>

void sendNextionCommand(String command);
void updateNextionText(String objectName, String text);
void updateNextionValue(String objectName, int value);
