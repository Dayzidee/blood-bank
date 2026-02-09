#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

struct Config {
  float  offsetC = 0.0;
  float  lowC    = 2.0;
  float  highC   = 6.0;
  String phones  = "";
  bool   soundEnable = true;
  bool   callEnable  = false;
  uint16_t smsIntervalMin = 5;
  // --- NEW: Cloud Config ---
  String wifi_ssid = "";
  String wifi_pass = "";
  String fb_url    = ""; 
  String fb_auth   = "";
};

extern Config cfg;

void loadConfig();
void saveConfig();
void loadDefaultsAndSave();

#endif
