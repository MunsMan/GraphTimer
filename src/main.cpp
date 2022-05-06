#include "core.h"

#include <Arduino.h>

const int BUTTON_PIN = D6;
const int LED_PIN = D1;

config_t config;
queue_t taskQueue;

IRAM_ATTR void coffeeISR() {
    Serial.flush();
    taskQueue.push(coffee_task);
}

void setup() {
    Serial.begin(115200);
    initConfig(&config, BUTTON_PIN, LED_PIN, coffeeISR);
    Serial.println("\nSetup finished!");
}

void loop() { taskHandler(&config, &taskQueue); }