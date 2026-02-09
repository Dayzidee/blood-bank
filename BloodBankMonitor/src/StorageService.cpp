#include "StorageService.h"
#include "Globals.h"
#include <LittleFS.h>

File logFile;
bool fsOk = false;

bool fsInit(){ 
  if(fsOk) return true; 
  fsOk = LittleFS.begin(true); 
  return fsOk; 
}

void openLogAppend(){
  if(!fsInit()) return;
  if(logFile) logFile.close();
  if(!LittleFS.exists("/log.csv")){
    File f=LittleFS.open("/log.csv",FILE_WRITE);
    if(f){ f.println("timestamp,tempC,alarm"); f.close(); }
  }
  logFile=LittleFS.open("/log.csv",FILE_APPEND);
}

void rotateLogIfNeeded(){
  if(!fsInit()) return; File f=LittleFS.open("/log.csv",FILE_READ); if(!f) return;
  size_t sz=f.size(); f.close(); if(sz<LOG_MAX_BYTES) return;
  LittleFS.remove("/log_old.csv"); LittleFS.rename("/log.csv","/log_old.csv"); openLogAppend();
}

String uptimeHMS(uint32_t ms){
  uint32_t s=ms/1000, h=s/3600, m=(s%3600)/60, ss=s%60;
  char b[16]; snprintf(b,sizeof(b),"%02u:%02u:%02u",(unsigned)h,(unsigned)m,(unsigned)ss);
  return String(b);
}

String currentTimestampLabel(){
  if(lastTimeStr.length()) return lastTimeStr;
  return String("UP ")+uptimeHMS(millis());
}

void appendLogRow(float t, bool alarm){
  if(!fsInit()) return; if(!logFile) openLogAppend(); if(!logFile) return;
  logFile.print(currentTimestampLabel()); logFile.print(",");
  logFile.print(t,3); logFile.print(",");
  logFile.println(alarm?1:0);
  logFile.flush();
}

void streamLogToClient(WebServer &server){
  if(!fsInit()){ server.send(500, "text/plain", "FS Error"); return; }
  
  // We send headers manually for a multi-file stream
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv", "");

  if(LittleFS.exists("/log_old.csv")){
    File f = LittleFS.open("/log_old.csv", FILE_READ);
    if(f){
      server.streamFile(f, "text/csv");
      f.close();
    }
  }

  if(LittleFS.exists("/log.csv")){
    File f = LittleFS.open("/log.csv", FILE_READ);
    if(f){
      server.streamFile(f, "text/csv");
      f.close();
    }
  }
}

void streamRecentLog(WebServer &server, size_t maxBytes){
  if(!fsInit()){ server.send(500, "text/plain", "FS Error"); return; }
  
  // We send headers for a fresh CSV response
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv", "");
  server.sendContent("timestamp,tempC,alarm\n");

  File f = LittleFS.open("/log.csv", FILE_READ);
  if(!f){ server.sendContent(""); return; } // Terminate empty
  
  size_t sz = f.size();
  if(sz > maxBytes) {
    f.seek(sz - maxBytes);
    // Skip the first partial line
    (void)f.readStringUntil('\n');
  }

  server.streamFile(f, "text/csv");
  f.close();
  server.sendContent(""); // End chunked if applicable
}
