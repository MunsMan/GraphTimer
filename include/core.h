#include "secrets.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <queue>

#define INIT_SINGLE_ESPRESSO_TIME 6000
#define INIT_DOUBLE_ESPRESSO_TIME 14000

#ifndef PERSONAL_SECRETS
#define WIFI_SSID = "YOUR WIFI SSID"
#define WIFI_PASSWORD = "YOUR WIFI PASSWORD"
#endif

#ifndef STORE_ADDR
#define STORE_ADDR 0
#endif

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_SETUP_RETRY 50

#define WIFI_ONLINE_TIME_SEC 120

typedef ESP8266WebServer webserver_t;

typedef struct config_t {
    unsigned long singleEspressoTime_ms;
    unsigned long doubleEspressoTime_ms;
    unsigned long countEspresso;
    int outputPin;
    int inputPin;
} config_t;

typedef struct store_t {
    unsigned long singleEspressoTime_ms;
    unsigned long doubleEspressoTime_ms;
} store_t;

typedef void (*callback_t)(config_t*);

typedef std::queue<callback_t> queue_t;

void initConfig(config_t* config, int inputPin, int outputPin, void (*ISR)());

void updateConfig(config_t* config);
void loadConfig(config_t* config);

void taskHandler(config_t* config, queue_t* queue, webserver_t* server);

void light_sleep();

/**
List of default Tasks

All Task have the callback_t type and recieve the service config.
**/

void coffee_task(config_t* config);

int setupWiFI();
void setupWebserver();