#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <Arduino.h>

#include <WebServer.h>

bool fsInit();
void openLogAppend();
void rotateLogIfNeeded();
String uptimeHMS(uint32_t ms);
String currentTimestampLabel();
void appendLogRow(float t, bool alarm);
void streamLogToClient(WebServer &server);
void streamRecentLog(WebServer &server, size_t maxBytes = 4096);

#endif
