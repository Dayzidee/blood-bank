#include "DisplayService.h"
#include "Globals.h"
#include "ConfigManager.h"
#include <Wire.h>
#include <WiFi.h>

void initDisplay(){
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

void lcdSplash(){
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Blood Bank Monitor");
  lcd.setCursor(0,1); lcd.print("AP: BloodBankTemp");
  lcd.setCursor(0,2); lcd.print("Pass: 12345678");
  lcd.setCursor(0,3); lcd.print("Open: 192.168.4.1");
}

void lcdScreen(){
  lcd.setCursor(0,0);
  const char* sState=(alarmState==ALARM)?"ALARM":(alarmState==PREALARM)?"PRE":" OK ";
  char l0[24]; snprintf(l0,sizeof(l0),"Temp %6.2f C  %s",tempFilt,sState);
  lcd.print(l0); for(int i=strlen(l0); i<20; i++) lcd.print(' ');

  lcd.setCursor(0,1);
  char l1[24]; snprintf(l1,sizeof(l1),"Range %4.1f-%4.1f C",cfg.lowC,cfg.highC);
  lcd.print(l1); for(int i=strlen(l1); i<20; i++) lcd.print(' ');

  lcd.setCursor(0,2);
  char l2[24]; snprintf(l2,sizeof(l2),"Min %5.1f Max %5.1f",minC,maxC);
  lcd.print(l2); for(int i=strlen(l2); i<20; i++) lcd.print(' ');

  lcd.setCursor(0,3);
  String net = WiFi.status()==WL_CONNECTED ? "W+" : "W-";
  bool cloudOk = (lastCloudSuccess > 0 && (millis()-lastCloudSuccess < 30000));
  String cld = cloudOk ? "C+" : "C-";
  char l3[21]; snprintf(l3,sizeof(l3),"%s %s S %s", net.c_str(), cld.c_str(), lastSmsInfo.c_str());
  lcd.print(l3); for(int i=strlen(l3); i<20; i++) lcd.print(' ');
}
