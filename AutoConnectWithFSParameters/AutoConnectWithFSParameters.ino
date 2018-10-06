/**
* @file AutoconnectWithFSParameters.ino
* @author Ihor Berizka
* @date 30 Sep 2018
* @brief File contains entry point to application.
*/

#include "wifiConfig.h"
#include "mqttConfig.h"
#include "adc_routine.h"

void setup() {

  pinMode(BTN_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), handleIntr, HIGH);
  pinMode(LED_RESET, OUTPUT);
  pinMode(LED_WIFI_OK, OUTPUT);
  pinMode(LED_WIFI_ERROR, OUTPUT);
  
  digitalWrite(LED_WIFI_ERROR, HIGH);
  //digitalWrite(LED_WIFI_OK, LOW);
  
  Serial.begin(115200);
  configWifi();
  Serial.println("stop AP mode");
  WiFi.softAPdisconnect();
  Serial.println("Connected to wifi.\n Start second machine state");
  digitalWrite(LED_WIFI_ERROR, LOW);
  digitalWrite(LED_WIFI_OK, HIGH);

  configMQTT();
  
  Serial.println("Start third machine state.");
}

void loop() {
  // put your main code here, to run repeatedly:
  //client.loop();
  //processADC();
  //delay(1000);
  clientmqtt.publish("success!", "Hello world!");
  delay(1000);
  resetConfigs();
  delay(100);
}

