#include "CloudService.h"
#include "Globals.h"
#include "ConfigManager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

WiFiClientSecure secureClient;

void initCloud(){
  secureClient.setInsecure(); // Skip certificate check for simplicity on ESP32
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void loopCloud(){
  // Only push if we have config and wifi
  if(cfg.wifi_ssid.length() < 2 || cfg.fb_url.length() < 10) return;
  if(WiFi.status() != WL_CONNECTED) return;
  
  if(millis() - lastCloudPush >= CLOUD_INTERVAL){
    lastCloudPush = millis();
    
    // Construct JSON payload
    // { "temp": 24.5, "low": 2.0, "high": 6.0, "min": 1.5, "max": 7.1, "alarm": false, "lastUpdate": 173456... }
    time_t now = time(nullptr);
    String json = "{";
    json += "\"temp\":" + String(tempFilt, 2) + ",";
    json += "\"low\":" + String(cfg.lowC, 1) + ",";
    json += "\"high\":" + String(cfg.highC, 1) + ",";
    json += "\"min\":" + String(minC, 2) + ",";
    json += "\"max\":" + String(maxC, 2) + ",";
    json += "\"alarm\":" + String(alarmState == ALARM ? "true" : "false") + ",";
    json += "\"sound\":" + String(cfg.soundEnable ? "1" : "0") + ",";
    json += "\"call\":" + String(cfg.callEnable ? "1" : "0") + ",";
    json += "\"phones\":\"" + cfg.phones + "\",";
    json += "\"smsmin\":" + String(cfg.smsIntervalMin) + ",";
    json += "\"lastUpdate\":" + String((unsigned long)now);
    json += "}";

    // Target: <fb_url>/device.json?auth=<token>
    String url = cfg.fb_url;
    if(url.endsWith("/")) url = url.substring(0, url.length()-1);
    
    String fullPath = url + "/device.json?auth=" + cfg.fb_auth;
    
    HTTPClient http;
    http.begin(secureClient, fullPath);
    http.addHeader("Content-Type", "application/json");
    
    int code = http.PATCH(json);
    if(code > 0){
      lastCloudSuccess = millis();
      if(code != 200 && code != 204) Serial.printf("Cloud Push HTTP: %d\n", code);
    } else {
      Serial.printf("Cloud Push FAIL: %s\n", http.errorToString(code).c_str());
    }
    http.end();
  }

  // --- Pull settings from cloud every 10 seconds ---
  static unsigned long lastCloudPull = 0;
  if(millis() - lastCloudPull >= 10000UL || lastCloudPull == 0){
    lastCloudPull = millis();
    
    String url = cfg.fb_url;
    if(url.endsWith("/")) url = url.substring(0, url.length()-1);
    String fullPath = url + "/settings.json?auth=" + cfg.fb_auth;
    
    HTTPClient http;
    http.begin(secureClient, fullPath);
    int code = http.GET();
    
    if(code == 200){
      lastCloudSuccess = millis();
      String payload = http.getString();
      if(payload.length() > 5 && payload != "null"){
        // Improved manual JSON parse (robust to spaces)
        auto getF = [&](String key) {
           int p = payload.indexOf("\""+key+"\"");
           if(p<0) return -999.0f;
           int colon = payload.indexOf(":", p);
           if(colon<0) return -999.0f;
           int q = payload.indexOf(",", colon);
           if(q<0) q = payload.indexOf("}", colon);
           return payload.substring(colon+1, q).toFloat();
        };
        auto getS = [&](String key) {
           int p = payload.indexOf("\""+key+"\"");
           if(p<0) return String("");
           int colon = payload.indexOf(":", p);
           if(colon<0) return String("");
           int start = payload.indexOf("\"", colon);
           if(start<0) return String("");
           int end = payload.indexOf("\"", start+1);
           if(end<0) return String("");
           return payload.substring(start+1, end);
        };
        
        float rLow = getF("low");
        float rHigh = getF("high");
        int rSnd = (int)getF("sound");
        String rPhones = getS("phones");
        int rSmsMin = (int)getF("smsmin");

        bool changed = false;
        if(rLow > -100.0 && rLow != cfg.lowC) { cfg.lowC = rLow; changed = true; }
        if(rHigh > -100.0 && rHigh != cfg.highC) { cfg.highC = rHigh; changed = true; }
        if(rSnd >= 0) {
            bool s = (rSnd == 1);
            if(s != cfg.soundEnable) { cfg.soundEnable = s; changed = true; }
        }
        int rCall = (int)getF("call");
        if(rCall >= 0) {
            bool c = (rCall == 1);
            if(c != cfg.callEnable) { cfg.callEnable = c; changed = true; }
        }
        if(rPhones.length() > 0 && rPhones != cfg.phones) { cfg.phones = rPhones; changed = true; }
        if(rSmsMin > 0 && rSmsMin != cfg.smsIntervalMin) { cfg.smsIntervalMin = rSmsMin; changed = true; }

        if(changed){
          Serial.println("Cloud Sync: Settings updated from Remote");
          extern void saveConfig(); // From ConfigManager
          saveConfig();
        }
      }
    } else if(code > 0){
      Serial.printf("Cloud Pull HTTP: %d\n", code);
    } else {
      Serial.printf("Cloud Pull FAIL: %s\n", http.errorToString(code).c_str());
    }
    http.end();
  }

  // --- Push history points every 15 minutes ---
  static unsigned long lastHistoryPush = 0;
  if(WiFi.status() == WL_CONNECTED && (millis() - lastHistoryPush >= 900000UL || lastHistoryPush == 0)){
    time_t now = time(nullptr);
    if(now > 1000000000L) { // Ensure time is synced
      lastHistoryPush = millis();
      String historyJson = "{";
      historyJson += "\"t\":" + String(tempFilt, 2) + ",";
      historyJson += "\"a\":" + String(alarmState == ALARM ? "1" : "0");
      historyJson += "}";

      String url = cfg.fb_url;
      if(url.endsWith("/")) url = url.substring(0, url.length()-1);
      String fullPath = url + "/history/" + String((unsigned long)now) + ".json?auth=" + cfg.fb_auth;

      HTTPClient http;
      http.begin(secureClient, fullPath);
      int code = http.PUT(historyJson); // Use PUT to create a node per timestamp
      if(code > 0) Serial.println("History Push OK");
      http.end();
    }
  }
}
