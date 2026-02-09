#include "SensorService.h"
#include "Globals.h"
#include <math.h>



template<typename T> T clampT_sensor(T v,T lo,T hi){return v<lo?lo:(v>hi?hi:v);}

void initSensor(){
  pinMode(THERM_ADC_PIN, INPUT);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
}

float adcRawToCelsius(int raw){
  raw = constrain(raw,1,ADC_MAX-1);
  float v = (raw*VREF)/ADC_MAX; v = clampT_sensor(v,0.001f,VREF-0.001f);
  float r = (v*THERM_R_FIXED)/(VREF-v); r = max(r,100.0f);
  float invT = (1.0/THERM_T0_K) + (1.0/THERM_BETA)*log(r/THERM_R0);
  return (1.0/invT) - 273.15;
}

float readNTC_Celsius_multi(int n){
  const int N=min(n,15); int buf[15];
  for(int i=0;i<N;i++){ buf[i]=analogRead(THERM_ADC_PIN); delayMicroseconds(250); }
  // median + trimmed mean
  for(int i=0;i<N;i++) for(int j=i+1;j<N;j++) if(buf[j]<buf[i]){int t=buf[i];buf[i]=buf[j];buf[j]=t;}
  int m=buf[N/2]; long sum=0; int cnt=0;
  for(int i=0;i<N;i++){ if(abs(buf[i]-m)<=100){ sum+=buf[i]; cnt++; } }
  int rawAvg = cnt? (int)(sum/cnt) : m;
  return adcRawToCelsius(rawAvg);
}

float filtTempMA(float x){
  avgBuf[avgIdx]=x; avgIdx=(avgIdx+1)%AVG_N; if(avgFill<AVG_N) avgFill++;
  float s=0; for(int i=0;i<avgFill;i++) s+=avgBuf[i]; return s/avgFill;
}
