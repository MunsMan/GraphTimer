#include "core.h"

#include <Arduino.h>
#include <EEPROM.h>

const int BUTTON_PIN = D8;
const int LED_PIN = D2;

config_t config;
queue_t taskQueue;
webserver_t http_rest_server(HTTP_REST_PORT);

IRAM_ATTR void coffeeISR() {
    Serial.flush();
    taskQueue.push(coffee_task);
}

void setup() {
    EEPROM.begin(sizeof(store_t));
    Serial.begin(115200);
    Serial.println();
    initConfig(&config, BUTTON_PIN, LED_PIN, coffeeISR);
    int wifiStatus = setupWiFI();
    if (wifiStatus == WL_CONNECTED) {
        setupWebserver();
    }
    Serial.println("\nSetup finished!");
}

void loop() { taskHandler(&config, &taskQueue, &http_rest_server); }