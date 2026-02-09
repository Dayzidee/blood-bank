#ifndef ALARM_SERVICE_H
#define ALARM_SERVICE_H

#include <Arduino.h>
#include "Globals.h"

void initAlarm();
bool outOfRange(float t);
void buzzerPatternUpdate();
void enterState(AlarmState s);

#endif
