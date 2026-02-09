#ifndef GSM_SERVICE_H
#define GSM_SERVICE_H

#include <Arduino.h>

void gsmInit();
void gsmPollTime();
void gsmSend(const String& s);
bool waitForChar(char ch, uint32_t ms = 7000);
String gsmReadAll(uint32_t ms = 200);
void sendSMS_one(const String& to, const String& body);
void broadcastSMS(const String& body);
void placeCallOnce();
String firstPhone();

#endif
