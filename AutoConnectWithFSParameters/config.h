#ifndef _CONFIG_
#define _CONFIG_
//wifi and mqtt configs
#define DEVICE_NAME_LEN 14u
#define WIFI_TIMEOUT 3u
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
//flag for saving data
bool shouldSaveConfig = false;

//define your default values here, if there are different values in config.json, they are overwritten.
/*char mqtt_server[SERVER_NAME_LEN] = "m23.cloudmqtt.com";
char mqtt_port[SERVER_PORT_LEN] = "12925";
char mqtt_user[USER_NAME_LEN] = "tlwhlgqr";
char mqtt_pwd[SERVER_PWD_LEN] = "g-VQc5c6w7eN";
char wifi_ssid[SSID_LEN] = "netis_2.4G_F4A82F";
char wifi_pwd[WIFI_PWD_LEN] = "281188semak";
*/
char wifiData[PARAM_NUM_WIFI][MAX_STRING_LEN] = {0};
char mqttData[PARAM_NUM_MQTT][MAX_STRING_LEN] = {0};
char buff[MAX_BUF_SIZE] = {0};
WiFiServer wifiServer(80);
const char device_name[DEVICE_NAME_LEN] = "ESP8266Client";
#endif

