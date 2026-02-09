#include "Globals.h"

const char* AP_SSID = "BloodBankTemp";
const char* AP_PASS = "12345678";

LiquidCrystal_I2C lcd(LCD_ADDR, 20, 4);
HardwareSerial GSM(2);

uint32_t LOG_WRITE_MS  = 60UL*1000UL;

AlarmState alarmState = NORMAL;
Preferences prefs;

float tempFilt = NAN;
float minC = NAN;
float maxC = NAN;

uint32_t lastSample = 0;
uint32_t lastLogWrite = 0;
uint32_t lastSmsMs = 0;
uint32_t lastCallMs = 0;

uint16_t outCnt = 0;
uint16_t inCnt = 0;

String lastSmsInfo = "none";
String lastCallInfo = "off";
String lastTimeStr = "";

// --- Cloud State ---
unsigned long lastCloudPush = 0;
unsigned long lastCloudSuccess = 0;
const unsigned long CLOUD_INTERVAL = 5000; // 5 seconds

// --- Moving Average ---
float avgBuf[AVG_N];
int avgIdx = 0;
int avgFill = 0;
