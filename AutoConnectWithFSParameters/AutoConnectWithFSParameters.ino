
#include "wifiConfig.h"
#include "mqttConfig.h"
#include "adc_routine.h"


//read analog pin - analogRead(A0);


void setup() {

  pinMode(BTN_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleIntr, HIGH);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  configWifi();
  Serial.println("Connected to wifi.\n Start second machine state");
  //configMQTT();
}

void loop() {
  // put your main code here, to run repeatedly:
  //client.loop();
  //processADC();
  //delay(1000);
  resetConfigs();
  delay(100);
}

