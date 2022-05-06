#include <core.h>

void initConfig(config_t* config, int inputPin, int outputPin, void (*ISR)()) {
    config->outputPin = outputPin;
    config->inputPin = inputPin;
    config->countEspresso = 0;
    config->singleEspressoTime_ms = INIT_SINGLE_ESPRESSO_TIME;
    config->doubleEspressoTime_ms = INIT_DOUBLE_ESPRESSO_TIME;
    pinMode(config->inputPin, INPUT);
    pinMode(config->outputPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(config->inputPin), ISR, RISING);
}

void coffee_task(config_t* config) {
    noInterrupts();
    unsigned long start_time = millis();
    unsigned long end_time = start_time + config->singleEspressoTime_ms;
    digitalWrite(config->outputPin, HIGH);
    bool low = false;
    bool doubleEspresso = false;
    Serial.println("Single Espresso");
    while (end_time > millis()) {
        if (!low && !digitalRead(config->inputPin) && !doubleEspresso) {
            low = true;
        }
        if (low && digitalRead(config->inputPin) && !doubleEspresso) {
            Serial.println("No a Double Espresso, please!");
            doubleEspresso = true;
            end_time = start_time + config->doubleEspressoTime_ms;
        }
    }
    digitalWrite(config->outputPin, LOW);
    interrupts();
}

void taskHandler(config_t* config, queue_t* queue) {
    if (!queue->empty()) {
        callback_t task = queue->front();
        task(config);
        queue->pop();
    }
    light_sleep();
}

void light_sleep() {}