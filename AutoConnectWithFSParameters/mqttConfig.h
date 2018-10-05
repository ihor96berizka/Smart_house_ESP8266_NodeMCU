#ifndef _MQTTCONFIG_
#define _MQTTCONFIG_
/**
 * @file mqttConfig.h
 * @author Ihor Berizka
 * @date 30 Sep 2018
 * @brief File contains functions related to MQTT config state.
*/
#include "wifiConfig.h"

#include <PubSubClient.h>

//mqtt related objects
WiFiClient espClient;
PubSubClient client(espClient);

void receiveDataCallback(char* topic, byte* payload, size_t length) {
 /* Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("qwerty");
    digitalWrite(ledPin, HIGH);   // Turn the LED on 
  } else {
    digitalWrite(ledPin, LOW);  // Turn the LED off 
    Serial.println("asdfg");
  }
*/
}

bool connectToMQTTServer()
{
  Serial.println("connectToMQTTServer func");
  client.setServer(mqttData[0], atoi(mqttData[1]));
  client.setCallback(receiveDataCallback);

  int start_time = millis();
  int end_time = millis();
 /* if (client.connect("ESP8266Client", mqtt_user, mqtt_pwd)) 
    {
      Serial.println("connected to mqtt server. return true");
      // Once connected, publish an announcement...
     // client.publish("hello", "hello world");
      // ... and resubscribe
      //client.subscribe("led");
      return true;
    } 
    else 
    {
      Serial.print("failed to connect to mqtt server. return false, rc=");
      Serial.print(client.state());
     // Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
    }
*/
  while (!client.connected()) 
  {
    if (client.connect("ESP8266Client", mqttData[2], mqttData[3]))
    {
    
    //delay(250);
    Serial.println("connected to mqtt");
    //end_time = millis();
    }
    else
    {
      Serial.println("Connection to mqtt failed.\n trying again...");
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
   }
  return true;  
}
void configMQTT()
{
  Serial.print("ConfigMQTT func");
  if (connectToMQTTServer())
  {
    return;
  }
  else
  {
   // startAP();
   // saveConfigToSPIFFS();
   // connectToMQTTServer();
   Serial.println("Coulnd not connect to mqtt server");
  }
  Serial.print("End of ConfigMQTT func");
}
#endif

