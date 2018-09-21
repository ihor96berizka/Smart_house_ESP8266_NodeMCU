//#include <ESP8266WiFi.h>
//#include <ESP8266WiFiAP.h>
#include <stdlib.h>
#include <WiFi.h>
char* ssid = "ESP8266_AP";//"ihor-GE62-6QD";
char* password = "11111111";
#define DATA_RECEIVED 0xFFu
#define HEADER_LEN 3u
#define WIFI_ERROR 10u

#define DELIMITER '\n'
#define MAX_STRING_LEN 25u
#define PARAM_NUM_MQTT 4u
#define PARAM_NUM_WIFI 2u
#define MAX_BUF_SIZE 100u
#define WIFI_CONFIG 3u
#define MQTT_CONFIG 4u

#define WIFI_TIMEOUT 3u

char wifiData[PARAM_NUM_WIFI][MAX_STRING_LEN] = {0};
char mqttData[PARAM_NUM_MQTT][MAX_STRING_LEN] = {0};
char buff[MAX_BUF_SIZE] = {0};
WiFiServer wifiServer(80);
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
void setup() {

  //buff = new unsigned char[DATA_LEN];
  Serial.begin(115200);
 
  delay(1000);
 
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
 
  wifiServer.begin();
}
 
void loop() {

 
  WiFiClient client = wifiServer.available();

  if (client) 
  {
     Serial.println("Client connected");
    //довжина корисних даних
    uint16_t DATA_LEN = 0;
    //тип даних - кофн wifi або mqtt
    uint8_t DATA_TYPE = 0;
    //тимчасовий буфер. на випадок якщо всі дані не прийшли відразу
    char* tmp_buf = nullptr;
    uint16_t msg_len = 0;
    //заголовок повідомлення. header[0] - DATA_TYPE, header[1] - DATA_LEN(MSB), header[2] - DATA_LEN(LSB)
    uint8_t header[HEADER_LEN];
    
    while (client.connected()) 
    {
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
      //якщо дійшли сюди - всі дані з сокета прочитані
     // Serial.println();
      //перевірити чи весь пакет прийшов. надіслати сповіщення клієнту що все отримано. 
      if (bytes_read > DATA_LEN)
      {
        client.write(DATA_RECEIVED);
        //отримано всі дані. вивести результат.
        Serial.println("\nraw Data: ");
        for (uint16_t i = 0; i < DATA_LEN; ++i)
        {
            Serial.print(char(buff[i]));  
        }
        
        Serial.println("Parsed data:");
        
        switch (header[0]) 
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
         case MQTT_CONFIG:
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
        default:
          break;
      } 

       delay(10);
      //спробувати підключитись до вайфаю. якщо не виходить - відправити код помилки.
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
      if (attempts < WIFI_TIMEOUT)
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
        Serial.println("clear wifi data...");
        clearData(PARAM_NUM_WIFI, wifiData); 
        memset(buff, '\0', MAX_BUF_SIZE); 
      }
      else
      {
        Serial.println("Failed to connect to wifi...\n Error message is sent to app");
        client.write(WIFI_ERROR);
        Serial.println("clear invalid wifi data...");
        clearData(PARAM_NUM_WIFI, wifiData);
        memset(buff, '\0', MAX_BUF_SIZE);
      }
   }//всі дані прочитано
    
   /* clearData(PARAM_NUM_WIFI, wifiData);
    Serial.println("widi data after clear...");
    for (uint8_t i = 0; i < PARAM_NUM_WIFI; ++i)
    {
        printLine(wifiData[i]);
        Serial.println();
    }
    */
   }//доки є клієнт(додаток підключений до есп)
    
    Serial.println();
   // client.stop();
    //Serial.println("Client disconnected");
    /*if (buff[DATA_LEN-1] == 'e')
      {
        Serial.println("Stopping AP and server...");
        wifiServer.stop();
        WiFi.softAPdisconnect();  
        Serial.println("Stopped AP and server...");
      }
  */ 
    
  }

   
 
}
