/*
  Blood Bank & Cold Room Monitor — Refactored
  ------------------------------------------------
  Main orchestration file for PlatformIO.
*/

#include <Arduino.h>
#include "Globals.h"
#include "ConfigManager.h"
#include "SensorService.h"
#include "DisplayService.h"
#include "GSMService.h"
#include "AlarmService.h"
#include "StorageService.h"
#include "WebInterface.h"
#include "CloudService.h"

void setup(){
  Serial.begin(115200);
  
  // Foundation
  loadConfig();
  fsInit();
  openLogAppend();
  rotateLogIfNeeded();
  
  // Hardware
  initAlarm();
  initSensor();
  initDisplay();
  lcdSplash();
  
  gsmInit();

  // Web & Cloud
  initWebInterface();
  initCloud();

  // Initial temperature sync
  float t0 = readNTC_Celsius_multi(PER_READ_SAMPLES) + cfg.offsetC;
  for(int i=0; i<AVG_N; i++) {
    filtTempMA(t0); 
  }
  tempFilt = minC = maxC = t0;
  
  delay(1000);
  lcd.clear();
}

void loop(){
  server.handleClient();
  gsmPollTime();
  loopCloud();

  if(millis() - lastSample >= SAMPLE_MS){
    lastSample = millis();
    
    // 1. Read Sensor
    float tRaw = readNTC_Celsius_multi(PER_READ_SAMPLES) + cfg.offsetC;
    tempFilt = filtTempMA(tRaw);

    // 2. Track Min/Max
    if(isnan(minC) || tempFilt < minC) minC = tempFilt;
    if(isnan(maxC) || tempFilt > maxC) maxC = tempFilt;

    // 3. Alarm Logic
    bool out = outOfRange(tempFilt);
    if(out){ outCnt++; inCnt=0; } else { inCnt++; outCnt=0; }

    if(alarmState == NORMAL   && outCnt >= PREALARM_SEC) enterState(PREALARM);
    if(alarmState == PREALARM && outCnt >= ALARM_SEC)   enterState(ALARM);
    if((alarmState == PREALARM || alarmState == ALARM) && inCnt >= CLEAR_SEC){
      alarmState = NORMAL; 
      lastSmsInfo = "cleared"; 
      lastCallInfo = "cleared";
    }

    // 4. GSM Repeating Alerts
    if(alarmState == ALARM){
      uint32_t interval_ms = (uint32_t)cfg.smsIntervalMin * 60UL * 1000UL;
      if(millis() - lastSmsMs >= interval_ms){
        broadcastSMS("REMINDER: Temp="+String(tempFilt,2)+"C outside "+String(cfg.lowC,1)+"-"+String(cfg.highC,1)+"C");
        lastSmsMs = millis();
      }
    }

    // 5. Update UI
    lcdScreen();

    // 6. Data Logging
    if(millis() - lastLogWrite >= LOG_WRITE_MS){
      lastLogWrite = millis(); 
      rotateLogIfNeeded(); 
      appendLogRow(tempFilt, (alarmState == ALARM));
    }
  }

  // Active Buzzer Pattern
  buzzerPatternUpdate();
}
