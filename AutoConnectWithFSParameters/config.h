#ifndef _CONFIG_
#define _CONFIG_

#include "Arduino.h"
//#include "FS.h"//
#include "SPIFFS.h"//for esp32

//wifi and mqtt configs
#define DEVICE_NAME_LEN 14u
#define WIFI_TIMEOUT 5u
#define DATA_RECEIVED 0xFFu
#define HEADER_LEN 3u
#define WIFI_ERROR 10u
#define MQTT_ERROR 20u

//esp32 ap name and pwd
char* ssid = "ESP32_AP";//"ihor-GE62-6QD";
char* password = "";

#define DELIMITER '\n'
#define MAX_STRING_LEN 25u
#define PARAM_NUM_MQTT 4u
#define PARAM_NUM_WIFI 2u
#define MAX_BUF_SIZE 100u
#define WIFI_CONFIG 3u
#define MQTT_CONFIG 4u

//ADC - related constants
#define REF_RESISTANCE 33
#define MIN_DATA_AMP_LIM 4
#define MAX_DATA_AMP_LIM 20
#define K_DIV (100.0 / 320.0)
#define MIN_VOLTAGE (REF_RESISTANCE * MIN_DATA_AMP_LIM)

#define RANGE_MV ((MAX_DATA_AMP_LIM * REF_RESISTANCE) - (MIN_DATA_AMP_LIM * REF_RESISTANCE))
#define RANGE_M 7 

char wifiData[PARAM_NUM_WIFI][MAX_STRING_LEN] = {0};
char mqttData[PARAM_NUM_MQTT][MAX_STRING_LEN] = {0};
char buff[MAX_BUF_SIZE] = {0};
WiFiServer wifiServer(80);
const char device_name[DEVICE_NAME_LEN] = "ESP8266Client";

#define BTN_PIN 35u
#define LED_RESET 27u
#define LED_WIFI_OK 14u
#define LED_WIFI_ERROR 12u
uint8_t state = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void clearData(uint8_t number_of_params, char output[][MAX_STRING_LEN])
{
  for (uint8_t i = 0; i < number_of_params; ++i)
  {
    memset(output[i],'\0',MAX_STRING_LEN);
  }  
}

void IRAM_ATTR handleIntr()
{
  portENTER_CRITICAL_ISR(&mux);
  state = 1;
  portEXIT_CRITICAL_ISR(&mux);
  //ESP.restart();
}

void resetConfigs()
{
  if (state)
  {
      digitalWrite(LED_WIFI_ERROR, LOW);
      digitalWrite(LED_WIFI_OK, LOW);
      digitalWrite(LED_RESET, 1);  
  
      Serial.println("Reset button pressed.");
      Serial.println("Clearing wifi and mqtt configs...");
      memset(buff, '\0', MAX_BUF_SIZE);
      Serial.println("wifi clear");
      clearData(PARAM_NUM_WIFI, wifiData);
      Serial.println("mqtt clear");
      clearData(PARAM_NUM_MQTT, mqttData);
      Serial.println("formatting flash");
      SPIFFS.format();
      Serial.println("Done!\nturn off led");
      delay(100);
      digitalWrite(LED_RESET, 0);
      Serial.println("Restart ESP module...");
      state = 0;
      ESP.restart();
  }  
}
#endif

