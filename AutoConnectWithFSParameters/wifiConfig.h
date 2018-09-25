#ifndef _WIFICONFIG_F
#define _WIFICONFIG_F

#include "Arduino.h"

//#include <FS.h>    for esp8266                //this needs to be first, or it all crashes and burns...
#include "FS.h"//
#include "SPIFFS.h"//for esp32
#include <WiFi.h>          //https://github.com/esp8266/Arduino

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "config.h"
//WiFiServer wifiServer(80);

void clearData(uint8_t number_of_params, char output[][MAX_STRING_LEN])
{
  for (uint8_t i = 0; i < number_of_params; ++i)
  {
    memset(output[i],'\0',MAX_STRING_LEN);
  }  
}
void printLine(const char* data)
{
  uint16_t len = strlen(data);
  for (uint8_t i = 0; i < len; ++i)
  {
    Serial.print(char(data[i]));  
  }  
}
bool readParams(const char* data, uint8_t number_of_params, char output[][MAX_STRING_LEN])
{
    uint8_t processed_params = 0;
    uint16_t begin = 0;
    uint16_t end = strlen(data);
    uint16_t current = begin;
    uint16_t word_begin = begin;
    uint16_t word_end = current;
    while (processed_params < number_of_params)
    {
        word_begin = current;
        //пройтись по всьому рядку до першого delimiter
        while(data[current] != DELIMITER && current != end + 1)
        {
            ++current;
        }
        word_end = current;
        //current буде вказувати на розділювач
        //якщо пройшлись по всьому і нема розділювача - invalid data
        if (current == end)
            return false;
        //скопіювати параметр
        memcpy(output[processed_params], data + word_begin, word_end - word_begin);
        ++current;
        ++processed_params;
    }
    return true;
}

void startAP()
{
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
 
  wifiServer.begin();
}
bool readSocket(WiFiClient& client, uint16_t& DATA_LEN, uint8_t& DATA_TYPE)
{
  bool finished = false;
  //тимчасовий буфер. на випадок якщо всі дані не прийшли відразу
    char* tmp_buf = nullptr;
    uint16_t msg_len = 0;
    //заголовок повідомлення. header[0] - DATA_TYPE, header[1] - DATA_LEN(MSB), header[2] - DATA_LEN(LSB)
    uint8_t header[HEADER_LEN];
   //для того щоб знати чи вже прочитали весь заголовок
      int bytes_read = 0;
   while (client.available() > 0) 
      {
        bytes_read++;
        //вичитати заголовок
        if (bytes_read <= HEADER_LEN)
        {
          header[bytes_read-1] = client.read();
          Serial.print(header[bytes_read-1]);
          //якщо прочитали заголовок. сформувати довжину буферу та тип
          if (bytes_read == HEADER_LEN)
          {
            DATA_TYPE = header[0];
            DATA_LEN = header[1] << 8 | header[2];
            Serial.println("\nHeader received!");
            Serial.print("Data len: ");
            Serial.println(DATA_LEN);
           
          }
          
        }//читати дані(значення параметрів)
        else
        {
            msg_len = client.available();
            Serial.print("bytes avaiable(msg_len): ");
            Serial.println(msg_len);
            if (tmp_buf == nullptr)
            {
              tmp_buf = new char[msg_len]; 
              //вичитати всі доступні байти
              bytes_read += client.read(reinterpret_cast<uint8_t*>(tmp_buf), msg_len); 
              Serial.println("received(tmp_buf): ");
              for (uint16_t i = 0; i < msg_len; ++i)
              {
                Serial.print(char(tmp_buf[i]));  
              }
              Serial.println();
              Serial.print("bytes_read = ");
              Serial.println(bytes_read);
              Serial.println("Before memcpy");
              memcpy(buff , tmp_buf, msg_len);
              Serial.println("After memcpy");
            }
            Serial.println("Before delete tmp_buf");
            delete[] tmp_buf;
            tmp_buf = nullptr;
            Serial.println("After delete tmp_buf");
        }
    
      }
      if (bytes_read > DATA_LEN)
      {
        finished = true;  
      }

  
  return finished;
}

bool connectToWifi()
{
  WiFi.begin(wifiData[0], wifiData[1]);
  Serial.print("Connecting to ");
  Serial.print(wifiData[0]); Serial.println(" ...");

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < WIFI_TIMEOUT) 
  { // Wait for the Wi-Fi to connect
     delay(1000);
     Serial.print("...");
     attempts++;
  }
  if (attempts < WIFI_TIMEOUT && WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0))
  {
        //client.stop();
        //Serial.println("Client disconnected");
       // Serial.println("Stopping AP and server...");
        //wifiServer.stop();
       // WiFi.softAPdisconnect();  
        //Serial.println("Stopped AP and server...");
        //Serial.println('\n');
      Serial.println("Connection established!");  
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());  
      //Serial.println("clear wifi data...");
      //clearData(PARAM_NUM_WIFI, wifiData); 
      memset(buff, '\0', MAX_BUF_SIZE);
      return true; 
   }
   else
   {
      Serial.println("Failed to connect to wifi...\n Error message is sent to app");
      //client.write(WIFI_ERROR);
      Serial.println("clear invalid wifi data...");
      clearData(PARAM_NUM_WIFI, wifiData);
      memset(buff, '\0', MAX_BUF_SIZE);
      WiFi.disconnect();
      return false;
   }
}
bool getDataFromAP()
{
  startAP();

  while(1)
  {
    WiFiClient client = wifiServer.available();

    if (client) 
    {
     Serial.println("Client connected");
    //довжина корисних даних
      uint16_t DATA_LEN = 0;
    //тип даних - кофн wifi або mqtt
      uint8_t DATA_TYPE = 0;
      while (client.connected()) 
      {
     // Serial.println();
      //перевірити чи весь пакет прийшов. надіслати сповіщення клієнту що все отримано. 
        if (readSocket(client, DATA_LEN, DATA_TYPE))
        {
          client.write(DATA_RECEIVED);
        //отримано всі дані. вивести результат.
          Serial.println("\nraw Data: ");
          for (uint16_t i = 0; i < DATA_LEN; ++i)
          {
            Serial.print(char(buff[i]));  
          }
        
          Serial.println("Parsed data:");
        
          switch (DATA_TYPE) 
          {
            case WIFI_CONFIG:
            {
          //call wifi parser
              Serial.println("WIFI settings:");
              readParams(buff, PARAM_NUM_WIFI, wifiData);
              Serial.println("Params parsed: ");
              for (uint8_t i = 0; i < PARAM_NUM_WIFI; ++i)
              {
                printLine(wifiData[i]);
                Serial.println();
              }
            }
            break;
        /*  case MQTT_CONFIG:
          {
        //call mqtt parser.
            Serial.println("\nMQTT settings:");
            readParams(buff, PARAM_NUM_MQTT, mqttData);
            Serial.println("Params parsed: ");
            for (uint8_t i = 0; i < PARAM_NUM_MQTT; ++i)
            {
              printLine(mqttData[i]);  
            }
          }
            break;
            */
          default:
            break;
        } 

        delay(10);
      //спробувати підключитись до вайфаю. якщо не виходить - відправити код помилки.
       if (connectToWifi())
       {
          return true; 
       }
       else
       {
          client.write(WIFI_ERROR); 
       }
    }//всі дані прочитано
    
   }//доки є клієнт(додаток підключений до есп)
 
  }
 }
  return false;
}
void saveConfigCallback()
{
  Serial.println("saveConfigCallBack func");
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void saveConfigToSPIFFS(uint8_t TYPE)
{
  Serial.println("SaveConfigToSPIFFS func");
  //save the custom parameters to FS
 
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    if (TYPE == WIFI_CONFIG)
    {
      json["wifi_ssid"] = wifiData[0];
      json["wifi_pwd"] = wifiData[1];      
    }
    else if (TYPE == MQTT_CONFIG)
    {
      json["mqtt_server"] = mqttData[0];
      json["mqtt_port"] = mqttData[1];
      json["mqtt_user"] = mqttData[2];
      json["mqtt_pwd"] = mqttData[3];  
    }
    
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
bool readConfigFromSPIFFS(uint8_t TYPE)
{
  Serial.println("readConfigFromSPIFFS");
  bool result = false;

  //clean FS, for testing
  //SPIFFS.format();
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
          if (TYPE == WIFI_CONFIG)
          {
            strcpy(wifiData[0], json["wifi_ssid"]);
            strcpy(wifiData[1], json["wifi_pwd"]);
          }
          else if (TYPE == MQTT_CONFIG)
          {
            strcpy(mqttData[0], json["mqtt_server"]);
            strcpy(mqttData[1], json["mqtt_port"]);
            strcpy(mqttData[2], json["mqtt_user"]);
            strcpy(mqttData[3], json["mqtt_pwd"]);  
          }
          result = true;
        } 
        else 
        {
          Serial.println("failed to load json config");
          result = false;
        }
      }
    }
  } else 
  {
    Serial.println("failed to mount FS");
    result = false;
  }
  //end read
  return result;
}
bool configWifi()
{
  Serial.println("Start of config wifi");
  if (readConfigFromSPIFFS(WIFI_CONFIG))
  {
    Serial.println("Data is fetched from flash");
    if (connectToWifi())
    {
      Serial.println("Connected to wifi.");
    }
    else
    {
      Serial.println("Invalid data in flash");  
      Serial.println("Failed to load data from flash.\nStart AP mode");
      if (getDataFromAP())
      {
        saveConfigToSPIFFS(WIFI_CONFIG);
        return true;
      }
      else
      {
        return false;  
      }
    }
  }
  else
  {
    Serial.println("Failed to load data from flash.\nStart AP mode");
    if (getDataFromAP())
    {
      saveConfigToSPIFFS(WIFI_CONFIG);
      return true;
    }
    else
    {
      return false;  
    }
  }
}


#endif

