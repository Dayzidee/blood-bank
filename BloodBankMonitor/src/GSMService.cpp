#include "GSMService.h"
#include "Globals.h"
#include "ConfigManager.h"

void gsmInit(){
  GSM.begin(GSM_BAUD,SERIAL_8N1,GSM_RX_PIN,GSM_TX_PIN);
  delay(1000); GSM.setTimeout(2000);
  
  // Keep trying to wake module up
  unsigned long t = millis();
  bool ready = false;
  while(millis() - t < 10000){
    gsmSend("AT\r"); 
    delay(200);
    if(gsmReadAll().indexOf("OK") >= 0) { ready = true; break; }
  }
  
  if(!ready) return; // Module not responding at all

  gsmSend("ATE0\r"); delay(200); gsmReadAll(); // Echo off
  gsmSend("AT+CMGF=1\r"); delay(200); gsmReadAll(); // Text mode
  gsmSend("AT+CSCS=\"GSM\"\r"); delay(200); gsmReadAll();
  gsmSend("AT+CNMI=0,0,0,0,0\r"); delay(200); gsmReadAll(); // No incoming SMS indications
}

bool gsmCheckNetwork(){
  // Check registration
  gsmSend("AT+CREG?\r");
  delay(200);
  String r = gsmReadAll();
  bool registered = (r.indexOf(",1") >= 0 || r.indexOf(",5") >= 0);

  // Check signal quality
  gsmSend("AT+CSQ\r");
  delay(200);
  String q = gsmReadAll(); // Format: +CSQ: 20,0
  
  // Debug output
  Serial.printf("GSM Status: Reg=%d Signal=%s", registered, q.c_str());
  if(!registered) Serial.println(" (Searching...)");

  return registered;
}

// Helper to wait for network with timeout
bool gsmWaitForNetwork(uint32_t timeoutMs){
  uint32_t start = millis();
  while(millis() - start < timeoutMs){
    if(gsmCheckNetwork()) return true;
    delay(1000); // Wait 1s between checks
  }
  return false;
}

void gsmPollTime(){
  static uint32_t lastGsmTimePoll = 0;
  const uint32_t GSM_TIME_POLL_MS = 60UL*1000UL;
  if(millis()-lastGsmTimePoll < GSM_TIME_POLL_MS) return;
  lastGsmTimePoll = millis();
  gsmSend("AT+CCLK?\r");
  delay(200);
  String r = gsmReadAll(400);
  int p = r.indexOf("+CCLK: ");
  if(p>=0){
    int q = r.indexOf('\n', p);
    String line = (q>p) ? r.substring(p, q) : r.substring(p);
    int q1 = line.indexOf('"'), q2 = line.indexOf('"', q1+1);
    if(q1>=0 && q2>q1){
      String ts = line.substring(q1+1, q2);
      if(ts.length() >= 17){
        String yy=ts.substring(0,2), MM=ts.substring(3,5), dd=ts.substring(6,8);
        String hh=ts.substring(9,11), mi=ts.substring(12,14), ss=ts.substring(15,17);
        lastTimeStr = String("20")+yy+"-"+MM+"-"+dd+" "+hh+":"+mi+":"+ss;
      } else {
        lastTimeStr = ts;
      }
    }
  }
}

void gsmSend(const String& s){ GSM.print(s); }

bool waitForChar(char ch, uint32_t ms){
  uint32_t t=millis();
  while(millis()-t<ms){ while(GSM.available()){ if(GSM.read()==ch) return true; } }
  return false;
}

String gsmReadAll(uint32_t ms){
  uint32_t t=millis(); String r;
  while(millis()-t<ms){ while(GSM.available()){ r += (char)GSM.read(); } }
  return r;
}

extern bool gsmWaitForNetwork(uint32_t ms); // Forward dec
void sendSMS_one(const String& to, const String& body){
  if(to.length()<3) return;
  
  // Try to find network for up to 10s
  if(!gsmWaitForNetwork(10000)){
    lastSmsInfo = "no-net";
    return;
  }

  // Ensure text mode before sending
  gsmSend("AT+CMGF=1\r"); delay(100); gsmReadAll();

  gsmSend("AT+CMGS=\""+to+"\"\r");
  // Increased wait for prompt to 10s (network leg)
  if(!waitForChar('>',10000)){ 
    lastSmsInfo="no-prompt"; 
    // Try to cancel if stuck
    GSM.write(27); // ESC
    return; 
  }
  gsmSend(body); GSM.write(26);
  lastSmsInfo="sent";
}

void broadcastSMS(const String& body){
  if(!cfg.phones.length()) return;
  int i=0; while(i<cfg.phones.length()){
    int c=cfg.phones.indexOf(',',i);
    String num=(c==-1)?cfg.phones.substring(i):cfg.phones.substring(i,c);
    num.trim(); if(num.length()>=3) sendSMS_one(num,body);
    if(c==-1) break; i=c+1;
  }
}

String firstPhone(){ String s=cfg.phones; s.trim(); int c=s.indexOf(','); if(c>0) s=s.substring(0,c); s.trim(); return s; }

void placeCallOnce(){
  if(!cfg.callEnable){ lastCallInfo="off"; return; }
  String n=firstPhone(); if(n.length()<3){ lastCallInfo="no-num"; return; }
  gsmSend("ATD"+n+";\r"); lastCallInfo="dialing";
  uint32_t t0=millis(); while(millis()-t0<20000) delay(10);
  gsmSend("ATH\r"); lastCallInfo="called";
}
