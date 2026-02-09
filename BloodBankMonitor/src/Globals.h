#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>

// --- Utils ---
template<typename T> T clampT(T v,T lo,T hi){return v<lo?lo:(v>hi?hi:v);}

// ---------- Wi-Fi AP ----------
extern const char* AP_SSID;
extern const char* AP_PASS;

// ---------- LCD 20x4 ----------
#define LCD_ADDR 0x27
extern LiquidCrystal_I2C lcd;

// ---------- NTC (10k, Beta 3950) on GPIO33 ----------
#define THERM_ADC_PIN 33
const float THERM_R_FIXED = 10000.0;
const float THERM_R0      = 10000.0;
const float THERM_T0_K    = 25.0 + 273.15;
const float THERM_BETA    = 3950.0;
const float VREF          = 3.3;
const int   ADC_MAX       = 4095;

// ---------- I/O ----------
#define BUZZER_PIN 27
#define RELAY_PIN  19
const int LEDC_CH=0, LEDC_RES=10, LEDC_DUTY=512;

// ---------- SIM900A (UART2) ----------
#define GSM_RX_PIN 16
#define GSM_TX_PIN 17
#define GSM_BAUD   9600
extern HardwareSerial GSM;

// ---------- Timing & logic ----------
const uint32_t SAMPLE_MS=1000;
const int      PER_READ_SAMPLES=16;
const uint8_t  PREALARM_SEC=5, ALARM_SEC=10, CLEAR_SEC=15;

extern uint32_t LOG_WRITE_MS;
const size_t LOG_MAX_BYTES = 1*1024*1024;

// ---------- Alarm state ----------
enum AlarmState { NORMAL=0, PREALARM=1, ALARM=2 };
extern AlarmState alarmState;
extern Preferences prefs;

// ---------- Global State Variables ----------
extern float tempFilt;
extern float minC;
extern float maxC;
extern uint32_t lastSample;
extern uint32_t lastLogWrite;
extern uint32_t lastSmsMs;
extern uint32_t lastCallMs;

extern uint16_t outCnt;
extern uint16_t inCnt;

extern String lastSmsInfo;
extern String lastCallInfo;
extern String lastTimeStr;

// --- Moving Average ---
const int AVG_N = 8;
extern float avgBuf[AVG_N];
extern int avgIdx;
extern int avgFill;

// --- NEW: Cloud State ---
extern unsigned long lastCloudPush;
extern unsigned long lastCloudSuccess;
extern const unsigned long CLOUD_INTERVAL;

#endif
