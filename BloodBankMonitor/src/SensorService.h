#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <Arduino.h>

void initSensor();
float adcRawToCelsius(int raw);
float readNTC_Celsius_multi(int n);
float filtTempMA(float x);

#endif
