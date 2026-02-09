#include "ConfigManager.h"
#include "Globals.h"
#include "Secrets.h"

Config cfg;

void loadConfig(){
  prefs.begin("bbtemp",true);
  cfg.offsetC = prefs.getFloat("ofsC",0.0);
  cfg.lowC    = clampT(prefs.getFloat("lowC",2.0f),-20.0f,40.0f);
  cfg.highC   = clampT(prefs.getFloat("highC",6.0f),-20.0f,40.0f);
  if(cfg.highC<=cfg.lowC){cfg.lowC=2.0; cfg.highC=6.0;}
  cfg.phones  = prefs.getString("phones","");
  cfg.soundEnable = prefs.getBool("sound",true);
  cfg.callEnable  = prefs.getBool("call",false);
  cfg.smsIntervalMin = clampT((uint16_t)prefs.getUShort("smsMin",5), (uint16_t)1, (uint16_t)1440);
  // New
  cfg.wifi_ssid = prefs.getString("wss","");
  cfg.wifi_pass = prefs.getString("wqs","");
  cfg.fb_url    = prefs.getString("fbu", FB_URL_DEFAULT);
  cfg.fb_auth   = prefs.getString("fba", FB_AUTH_DEFAULT);
  prefs.end();
}

void saveConfig(){
  prefs.begin("bbtemp",false);
  prefs.putFloat("ofsC",cfg.offsetC);
  prefs.putFloat("lowC",cfg.lowC);
  prefs.putFloat("highC",cfg.highC);
  prefs.putString("phones",cfg.phones);
  prefs.putBool("sound",cfg.soundEnable);
  prefs.putBool("call",cfg.callEnable);
  prefs.putUShort("smsMin",cfg.smsIntervalMin);
  // New
  prefs.putString("wss",cfg.wifi_ssid);
  prefs.putString("wqs",cfg.wifi_pass);
  prefs.putString("fbu",cfg.fb_url);
  prefs.putString("fba",cfg.fb_auth);
  prefs.end();
}

void loadDefaultsAndSave(){
  cfg.offsetC = 0.0;
  cfg.lowC    = 2.0;
  cfg.highC   = 6.0;
  cfg.phones  = "";
  cfg.soundEnable = true;
  cfg.callEnable  = false;
  cfg.smsIntervalMin = 5;
  cfg.wifi_ssid = "";
  cfg.wifi_pass = "";
  cfg.fb_url = FB_URL_DEFAULT;
  cfg.fb_auth = FB_AUTH_DEFAULT;
  saveConfig();
}
