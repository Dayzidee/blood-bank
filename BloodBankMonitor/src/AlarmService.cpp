#include "AlarmService.h"
#include "Globals.h"
#include "ConfigManager.h"
#include "GSMService.h"

void initAlarm(){
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  ledcSetup(LEDC_CH, 1000, LEDC_RES);
  ledcAttachPin(BUZZER_PIN, LEDC_CH);
  ledcWrite(LEDC_CH, 0);
}

bool outOfRange(float t){
  return t < cfg.lowC || t > cfg.highC;
}

void buzzerPatternUpdate(){
  static bool hi=false; static uint32_t toggleT=0; const uint32_t now=millis();
  if(!cfg.soundEnable){ ledcWrite(LEDC_CH,0); digitalWrite(RELAY_PIN,(alarmState==ALARM)?HIGH:LOW); return; }
  if(alarmState==PREALARM){
    const uint32_t period=1500,on=220;
    if((now%period)<on){ ledcWriteTone(LEDC_CH,1000); ledcWrite(LEDC_CH,LEDC_DUTY);}
    else ledcWrite(LEDC_CH,0);
    digitalWrite(RELAY_PIN,LOW);
  } else if(alarmState==ALARM){
    if(now-toggleT>=500){ toggleT=now; hi=!hi; ledcWriteTone(LEDC_CH,hi?880:660); ledcWrite(LEDC_CH,LEDC_DUTY); }
    digitalWrite(RELAY_PIN,HIGH);
  } else {
    ledcWrite(LEDC_CH,0); digitalWrite(RELAY_PIN,LOW);
  }
}

void enterState(AlarmState s){
  alarmState=s;
  if(s==ALARM){
    String msg="ALERT: Temp="+String(tempFilt,2)+"C (range "+String(cfg.lowC,1)+"-"+String(cfg.highC,1)+"C)";
    broadcastSMS(msg); lastSmsMs=millis();
    if(cfg.callEnable){ placeCallOnce(); lastCallMs=millis(); }
  }
}
