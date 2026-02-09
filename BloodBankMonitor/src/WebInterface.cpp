#include "WebInterface.h"
#include "WebPages.h"
#include "Globals.h"
#include "ConfigManager.h"
#include "StorageService.h"
#include "GSMService.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <Preferences.h>

WebServer server(80);

void handleInfo(){
  String j = "{";
  j += "\"temp\":"   + String(tempFilt,2) + ",";
  j += "\"low\":"    + String(cfg.lowC,2) + ",";
  j += "\"high\":"   + String(cfg.highC,2) + ",";
  j += "\"offset\":" + String(cfg.offsetC,2) + ",";
  j += "\"alarm\":";
  j += (alarmState==ALARM ? "true" : "false");
  j += ",";
  j += "\"min\":"    + String(minC,2) + ",";
  j += "\"max\":"    + String(maxC,2) + ",";
  j += "\"fbu\":\"" + cfg.fb_url + "\",";
  j += "\"time\":\"" + (lastTimeStr.length()? lastTimeStr : uptimeHMS(millis())) + "\",";
  j += "\"lastSms\":\""  + lastSmsInfo  + "\",";
  j += "\"lastCall\":\"" + lastCallInfo + "\"";
  j += "}";
  server.send(200,"application/json",j);
}

void handleGetCfg(){
  String j = "{";
  j += "\"temp\":"    + String(tempFilt,2) + ",";
  j += "\"offset\":"  + String(cfg.offsetC,2) + ",";
  j += "\"low\":"     + String(cfg.lowC,2) + ",";
  j += "\"high\":"    + String(cfg.highC,2) + ",";
  j += "\"wss\":\""  + cfg.wifi_ssid + "\",";
  j += "\"wqs\":\""  + cfg.wifi_pass + "\",";
  j += "\"fbu\":\""  + cfg.fb_url + "\",";
  j += "\"fba\":\""  + cfg.fb_auth + "\",";
  j += "\"phones\":\"" + cfg.phones + "\",";
  j += "\"sound\":"   + String(cfg.soundEnable ? "true" : "false") + ",";
  j += "\"call\":"    + String(cfg.callEnable  ? "true" : "false") + ",";
  j += "\"smsmin\":"  + String((int)cfg.smsIntervalMin);
  j += "}";
  server.send(200,"application/json",j);
}

void handleSaveCfg(){
  if(server.method()!=HTTP_POST){server.send(405,"text/plain","POST only");return;}
  if(server.hasArg("ofs"))    cfg.offsetC=clampT(server.arg("ofs").toFloat(),-5.0f,5.0f);
  if(server.hasArg("low"))    cfg.lowC   =clampT(server.arg("low").toFloat(),-20.0f,40.0f);
  if(server.hasArg("high"))   cfg.highC  =clampT(server.arg("high").toFloat(),-20.0f,40.0f);
  if(cfg.highC<=cfg.lowC){cfg.lowC=2.0; cfg.highC=6.0;}
  if(server.hasArg("phones")) cfg.phones = server.arg("phones");
  if(server.hasArg("wss"))    cfg.wifi_ssid = server.arg("wss");
  if(server.hasArg("wqs"))    cfg.wifi_pass = server.arg("wqs");
  
  cfg.soundEnable = server.hasArg("sound");
  cfg.callEnable  = server.hasArg("call");
  if(server.hasArg("smsmin")){
    int v = server.arg("smsmin").toInt();
    cfg.smsIntervalMin = clampT((uint16_t)(v>0?v:5), (uint16_t)1, (uint16_t)1440);
  }
  saveConfig(); 
  server.send(200,"text/plain","ok");
  delay(500); 
  ESP.restart(); 
}

void handleTestSMS(){ 
  broadcastSMS("Blood Bank Monitor TEST \u2014 T="+String(tempFilt,2)+"C"); 
  server.send(200,"text/plain","ok"); 
}

void handleSound(){
  if(server.hasArg("on")){ cfg.soundEnable=(server.arg("on")=="1"); saveConfig(); }
  server.send(200,"application/json",String("{\"sound\":")+(cfg.soundEnable?"true":"false")+"}");
}

void handleLogCsv(){ 
  streamLogToClient(server);
}

void handleRecentLog(){
  streamRecentLog(server, 4096); // Last 4KB is plenty for the chart
}

void handleExport(){ 
  server.sendHeader("Content-Disposition","attachment; filename=log.csv"); 
  streamLogToClient(server);
}

void handleResetMinMax(){ minC=tempFilt; maxC=tempFilt; server.send(200,"text/plain","ok"); }

void handleClearLog(){
  if(fsInit()){ 
    LittleFS.remove("/log.csv"); 
    LittleFS.remove("/log_old.csv"); 
    openLogAppend(); 
  }
  server.send(200,"text/plain","ok");
}

void handleFactoryReset(){
  prefs.begin("bbtemp", false);
  prefs.clear();
  prefs.end();
  loadDefaultsAndSave();
  server.send(200, "text/plain", "restarting");
  delay(250);
  ESP.restart();
}

void initWebInterface(){
  WiFi.mode(WIFI_AP_STA); 
  WiFi.softAP(AP_SSID,AP_PASS);
  
  if(cfg.wifi_ssid.length() >= 2){
    WiFi.begin(cfg.wifi_ssid.c_str(), cfg.wifi_pass.c_str());
    Serial.println("Connecting to STA: " + cfg.wifi_ssid);
  }

  server.on("/",            [](){ 
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent(htmlDashboard()); 
    server.sendContent(""); // Terminate chunked response
  });
  server.on("/settings",    [](){ server.send(200,"text/html",htmlSettings()); });
  server.on("/history",     [](){ server.send(200,"text/html",htmlHistory()); });
  // Fixed handleHistory to send response
  server.on("/api/info",    handleInfo);
  server.on("/api/getcfg",  handleGetCfg);
  server.on("/api/savecfg", HTTP_POST, handleSaveCfg);
  server.on("/api/testsms", handleTestSMS);
  server.on("/api/sound",   handleSound);
  server.on("/log.csv",     handleLogCsv);
  server.on("/api/recent_log", handleRecentLog);
  server.on("/export",      handleExport);
  server.on("/api/resetminmax", handleResetMinMax);
  server.on("/api/clearlog",    handleClearLog);
  server.on("/api/factoryreset",handleFactoryReset);

  server.onNotFound([](){ server.send(404,"text/plain","Not found"); });
  server.begin();
}
