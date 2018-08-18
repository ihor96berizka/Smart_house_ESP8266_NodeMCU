#ifndef _CONFIG_
#define _CONFIG_

#define SERVER_NAME_LEN 30u
#define SERVER_PORT_LEN 7u
#define USER_NAME_LEN 20u
#define SERVER_PWD_LEN 14u
#define SSID_LEN 20u
#define WIFI_PWD_LEN 10u
#define DEVICE_NAME_LEN 14u
#define WIFI_TIMEOUT 2000u
#define K_DIV (100.0 / 320.0)
//flag for saving data
bool shouldSaveConfig = false;

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[SERVER_NAME_LEN] = "m23.cloudmqtt.com";
char mqtt_port[SERVER_PORT_LEN] = "12925";
char mqtt_user[USER_NAME_LEN] = "tlwhlgqr";
char mqtt_pwd[SERVER_PWD_LEN] = "g-VQc5c6w7eN";
char wifi_ssid[SSID_LEN] = "ihor-GE62-6QD";
char wifi_pwd[WIFI_PWD_LEN] = "11111111";
char device_name[DEVICE_NAME_LEN] = "ESP8266Client";
#endif

