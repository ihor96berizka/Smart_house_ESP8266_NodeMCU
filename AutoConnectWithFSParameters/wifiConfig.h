/**
* @file wifiConfig.h
* @author Ihor Berizka
* @date 30 Sep 2018
* @brief File contains functions related to WiFi config state.
*/
#ifndef _WIFICONFIG_F
#define _WIFICONFIG_F

#include <WiFi.h>         

#include <ArduinoJson.h>          
#include "config.h"

/**
 * @brief Function used for debug. 
 * Prints input parameter to serial port.
 * @param data : C-style string to be printed.
*/
void printLine(const char* data)
{
  uint16_t len = strlen(data);
  for (uint8_t i = 0; i < len; ++i)
  {
    Serial.print(char(data[i]));  
  }  
}
/**
 * @brief Parses data fetched from socket.
 * @param data : C-style string to be parsed.
 * @param number_of_params : number of parameters in string.
 * @return output : array of C-style strings. Parsed parameters are saved in this parameter.
*/
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
        //find first delimiter
        while(data[current] != DELIMITER && current != end + 1)
        {
            ++current;
        }
        word_end = current;
        //current points to delimiter
        //if current equals to end - invalid data
        if (current == end)
            return false;
        //copy parameter
        memcpy(output[processed_params], data + word_begin, word_end - word_begin);
        ++current;
        ++processed_params;
    }
    return true;
}
/**
 * @brief Launches AP mode.
 * As a result - we can connect to ESP32 module via WiFi.
*/
void startAP()
{
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
 
  wifiServer.begin();
}
/**
 * @brief Fetch data from client.
 * @param client : client connected to TCP\IP server on ESP32.
 * @return DATA_LEN : number of bytes to read from socket.
 * @return DATA_TYPE : type of data packet.
 * @return SOCKET_NO_DATA if no data available.
 * @return SOCKET_INVALID_DATA if received invalid data packet.
 * @return SOCKET_CORRECT_DATA if received correct data packet.
*/
uint8_t readSocket(WiFiClient& client, uint16_t& DATA_LEN, uint8_t& DATA_TYPE)
{
   uint8_t finished = SOCKET_NO_DATA;
   //temporary buffer.For case when data is fetched in portions. 
   char* tmp_buf = nullptr;
   uint16_t msg_len = 0;
   //packet header. header[1] - DATA_TYPE, header[2] - DATA_LEN(MSB), header[3] - DATA_LEN(LSB)
   uint8_t header[HEADER_LEN];
   //number of read bytes.
   int bytes_read = 0;
   while (client.available() > 0) 
      {
        Serial.println("Bytes available > 0");
        bytes_read++;
        //read header
        if (bytes_read <= HEADER_LEN)
        {
          header[bytes_read-1] = client.read();
          Serial.println(header[bytes_read-1]);
          if (header[0] != DEVICE_CONFIG)
          {
            finished = SOCKET_INVALID_DATA;
            return finished;  
          }
          //parse packet type and payload size
          if (bytes_read == HEADER_LEN)
          {
            DATA_TYPE = header[1];
            DATA_LEN = header[2] << 8 | header[3];
            Serial.println("\nHeader received!");
            Serial.print("Data len: ");
            Serial.println(DATA_LEN);
           
          }
          
        }//read payload
        else
        {
            msg_len = client.available();
            Serial.print("bytes avaiable(msg_len): ");
            Serial.println(msg_len);
            if (tmp_buf == nullptr)
            {
              tmp_buf = new char[msg_len]; 
              //read all avaible bytes
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
        finished = SOCKET_CORRECT_DATA;  
      }

  return finished;
}

/**
 * @brief Connect to WiFi using saved SSID and password.
 * @return Return true if connection is successful
*/
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
      Serial.println("Connection established!");  
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP()); 
      memset(buff, '\0', MAX_BUF_SIZE);
      return true; 
   }
   else
   {
      Serial.println("Failed to connect to wifi...\n Error message is sent to app");
      Serial.println("clear invalid wifi data...");
      clearData(PARAM_NUM_WIFI, wifiData);
      memset(buff, '\0', MAX_BUF_SIZE);
      WiFi.disconnect();
      return false;
   }
}
/**
 * @brief Starts AP mode.
 * Gets data from client(app) and tryes to connect to WiFi.
 * If connection successful - returns true and writes to client WIFI_CONNECTED code  new IP address.
 * If connection failed - returns false and writes to client WIFI_ERROR code.
 * @return True if connected to WiFi. False otherwise.
*/
bool getDataFromAP()
{
  startAP();

  while(1)
  {
    WiFiClient client = wifiServer.available();

    if (client) 
    {
     Serial.println("Client connected");
    //payload size
      uint16_t DATA_LEN = 0;
    //payload type - wifi or mqtt config
      uint8_t DATA_TYPE = 0;
      while (client.connected()) 
      {
     // Serial.println();
      //перевірити чи весь пакет прийшов. надіслати сповіщення клієнту що все отримано. 
      uint8_t socketStatus = readSocket(client, DATA_LEN, DATA_TYPE);
        if (socketStatus == SOCKET_CORRECT_DATA)
        {
         // client.write(DATA_RECEIVED);
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
          client.write((uint8_t)(WIFI_CONNECTED));
          const char* currentIP = WiFi.localIP().toString().c_str();
          client.write(currentIP, strlen(currentIP));
          return true; 
       }
       else
       {
          client.write(WIFI_ERROR); 
       }
    }//всі дані прочитано
    else if (socketStatus == SOCKET_INVALID_DATA)//прийшло щось лєве... тут перевірка не канає.
    {
      Serial.println("Unsupported packet...");
      Serial.println("Client disconnected!");
      client.stop();
    }
    
   }//доки є клієнт(додаток підключений до есп)
 
  }
 }
  return false;
}

/**
 * @brief Saves fetched configs to internal flash memory
 * @param TYPE : indicates which config to save: WiFi or MQTT.
*/
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

/**
 * @brief Fetches saved configs from internal flash memory. 
 * @param TYPE : indicates which config to fetch: WiFi or MQTT.
 * @return True if fetched data successfully, false otherwise.
*/
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

/**
 * @brief Main function for WiFi configuration.
 * @return True if connected to WiFi. False otherrwise.
*/
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
