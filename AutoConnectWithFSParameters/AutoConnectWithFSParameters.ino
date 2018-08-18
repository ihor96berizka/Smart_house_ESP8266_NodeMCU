
#include "wifiConfig.h"
#include "mqttConfig.h"

#define MAX_PWM_VAL 1023
#define MSG_LEN 30u
//read analog pin - analogRead(A0);
char msg[MSG_LEN];

int sensorVal = 0;
double rangeVal = 0;
uint8_t ledPin = D5;

double mapToDistance(int voltage)
{
  return (voltage - 100) * (700.0 / 600);
} 
void setup() {

  Serial.begin(115200);
  configWifi();
  configMQTT();
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
  sensorVal = analogRead(A0);
  
  //map from ADC raw val to mV
   int mV = map(sensorVal, 0, 1023, 0, 3300);

   mV *= K_DIV;
  if (mV > 700)
    mV = 700;
  if (mV < 130)
    mV = 130;
    
    rangeVal = mapToDistance(mV);
    
    Serial.print("Currenr range: ");
    Serial.println(rangeVal);
    Serial.print("Currenr voltage: ");
    Serial.println(mV);
    snprintf (msg, MSG_LEN, "Currenr rangeVal: %.2lf сm", rangeVal);
    client.publish("Range", msg);
    analogWrite(ledPin, sensorVal);
    
    Serial.print("Currenr range:(char*) ");
    Serial.println(msg);
    delay(250);
  
}

