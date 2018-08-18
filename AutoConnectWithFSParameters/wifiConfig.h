#ifndef _WIFICONFIG_F
#define _WIFICONFIG_F

#include "Arduino.h"

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "config.h"

bool connectToWifi()
{
  Serial.println("connect to wifi func");
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_pwd);

  int start_time = millis();
  int end_time = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    if ((end_time - start_time) < WIFI_TIMEOUT)
    {
     delay(500);
     Serial.print(".");
     end_time = millis();
    }
    else
    {
      Serial.println("Connection to wifi failed.\n Returning false");
      return false;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected \nReturning true");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
void saveConfigCallback()
{
  Serial.println("saveConfigCallBack func");
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void startAP()
{
  Serial.println("Start AP func");
   // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "user", mqtt_user, 32);
  WiFiManagerParameter custom_mqtt_pwd("password", "password", mqtt_pwd, 12);


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pwd);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "ESP_AP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("ESP_AP")) 
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
 //if you get here you have connected to the WiFi
  Serial.println("Connected from AP mode to wifi\n"
  "Updating configs...");
  Serial.println("connected...yeey :)");
  
  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pwd, custom_mqtt_pwd.getValue());
  strcpy(wifi_ssid, wifiManager.getSSID());
  strcpy(wifi_pwd, wifiManager.getPassword());
}
void saveConfigToSPIFFS()
{
  Serial.println("SaveConfigToSPIFFS func");
  //save the custom parameters to FS
  if (shouldSaveConfig) 
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["wifi_ssid"] = wifi_ssid;
    json["wifi_pwd"] = wifi_pwd;
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pwd"] = mqtt_pwd;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("Saved parameters to SPIFFS");
    //end save
  }
  
}
bool readConfigFromSPIFFS()
{
  Serial.println("readConfigFromSPIFFS");
  bool result = false;
  
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  SPIFFS.format();
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) 
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) 
    {
      result = true;
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          
          strcpy(wifi_ssid, json["wifi_ssid"]);
          strcpy(wifi_pwd, json["wifi_pwd"]);
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pwd, json["mqtt_pwd"]);
        } else 
        {
          Serial.println("failed to load json config");
          result = false;
        }
      }
    }
  } else 
  {
    Serial.println("failed to mount FS");
  }
  //end read
  return result;
}
void configWifi()
{
  Serial.println("conigWIFI func");
  //може допомогти з нестабільною роботою в AP режимі
 // WiFi.setAutoConnect(false);
 // WiFi.setAutoReconnect(false);
  bool connected = false;
  //read config data from SPIFFS. 
  //If config file exists -> connect to wifi. 
  //else use default config values
  connected = connectToWifi();
  if (connected/*readConfigFromSPIFFS()*/)
  {
    //try to connect to wifi 
    //connected = connectToWifi();
    return;
  }
  else
  {
    //use default values for wifi config
    //if connection fails - start AP
   // connected = connectToWifi();
    if (!connected)
    {
     // startAP();
     // saveConfigToSPIFFS();
    }
  }
  Serial.println("End of configwifi func");
  Serial.println("\nlocal ip");
  Serial.println(WiFi.localIP());
}


#endif

