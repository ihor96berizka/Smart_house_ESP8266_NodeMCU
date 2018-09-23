#ifndef _ADC_ROUTINE_
#define _ADC_ROUTINE_

#include "Arduino.h"
#include "config.h"

#define MAX_PWM_VAL 1023
#define MSG_LEN 30u

double mapToDistance(uint16_t voltage)
{
  return ((double)voltage - (double)MIN_VOLTAGE) * ((double)(RANGE_M) / (double)(RANGE_MV));
} 
void processADC()
{
  uint16_t sensorVal = analogRead(A0);
  double rangeVal = 0;
  char msg[MSG_LEN];

  uint8_t ledPin = 1;//D5;
  //map from ADC raw val to mV
  uint16_t mV = map(sensorVal, 0, 1024, 0, 950);

  mV /= K_DIV;
  if (mV > 660)
  {
    mV = 660;
  } 
  if (mV < 132)
  {
   mV = 132;
  }
  
  rangeVal = mapToDistance(mV);
/*  Serial.print("Min voltage: ");
  Serial.println(MIN_VOLTAGE);

  Serial.print("range m: ");
  Serial.println(RANGE_M);

  
  Serial.print("range voltage: ");
  Serial.println(RANGE_MV);
  
  Serial.print("voltage: ");
  Serial.println(mV);
  
  Serial.print("range: ");
  Serial.println(rangeVal);
  */
  //in cm
  rangeVal *= 100;
  //Serial.print("Currenr range: ");
  //Serial.println(rangeVal);
  //Serial.print("Currenr voltage: ");
  //Serial.println(mV);
  snprintf (msg, MSG_LEN, "Current range: %.2lf cm", rangeVal);
  client.publish("range", msg);
  snprintf (msg, MSG_LEN, "Current bits: %ul", sensorVal);
  client.publish("bits", msg);
  snprintf (msg, MSG_LEN, "Current mV: %ul", mV);
  client.publish("voltage", msg);
  //analogWrite(ledPin, sensorVal);
    
  //Serial.print("Currenr range:(char*) ");
  //Serial.println(msg);
}
#endif

