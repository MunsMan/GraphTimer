#include <core.h>

extern webserver_t http_rest_server;
extern config_t config;

void setTimeHandler();
void getTimeHandler();
void getCoffeeCounterHandler();

void initConfig(config_t* config, int inputPin, int outputPin, void (*ISR)()) {
    config->outputPin = outputPin;
    config->inputPin = inputPin;
    config->countEspresso = 0;
    pinMode(config->inputPin, INPUT_PULLDOWN_16);
    pinMode(config->outputPin, OUTPUT);
    loadConfig(config);
    attachInterrupt(digitalPinToInterrupt(config->inputPin), ISR, RISING);
}

void loadConfig(config_t* config) {
    store_t store;
    EEPROM.get(STORE_ADDR, store);
    config->doubleEspressoTime_ms = store.doubleEspressoTime_ms;
    config->singleEspressoTime_ms = store.singleEspressoTime_ms;
}

void updateConfig(config_t* config) {
    store_t store;
    store.singleEspressoTime_ms = config->singleEspressoTime_ms;
    store.doubleEspressoTime_ms = config->doubleEspressoTime_ms;
    EEPROM.put(STORE_ADDR, store);
    EEPROM.commit();
}

void coffee_task(config_t* config) {
    noInterrupts();
    unsigned long start_time = millis();
    unsigned long end_time = start_time + config->singleEspressoTime_ms;
    while (start_time + 100 > millis()) {
        if (!digitalRead(config->inputPin)) {
            interrupts();
            return;
        }
    }
    digitalWrite(config->outputPin, HIGH);
    bool low = false;
    bool doubleEspresso = false;
    Serial.println("Single Espresso");
    config->countEspresso++;
    while (start_time + 500 > millis()) {
        if (!low && !digitalRead(config->inputPin)) {
            while (start_time + 50 > millis()) {
                if (digitalRead(config->inputPin)) {
                    continue;
                }
            }
            low = true;
        }
    }
    while (end_time > millis()) {
        wdt_reset();
        if (!low && !digitalRead(config->inputPin) && !doubleEspresso) {
            while (start_time + 50 > millis()) {
                if (digitalRead(config->inputPin)) {
                    continue;
                }
            }
            low = true;
        }
        if (low && digitalRead(config->inputPin) && !doubleEspresso) {
            while (start_time + 100 > millis()) {
                if (!digitalRead(config->inputPin)) {
                    continue;
                }
            }
            Serial.println("No a Double Espresso, please!");
            config->countEspresso++;
            doubleEspresso = true;
            end_time = start_time + config->doubleEspressoTime_ms;
        }
    }
    digitalWrite(config->outputPin, LOW);
    interrupts();
}

void taskHandler(config_t* config, queue_t* queue, webserver_t* server) {
    if (!queue->empty()) {
        while (!queue->empty()) {
            callback_t task = queue->front();
            task(config);
            queue->pop();
            server->handleClient();
        }
    }
    server->handleClient();
}

int setupWiFI() {
    int retries = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to WiFi AP");
    while ((WiFi.status() != WL_CONNECTED) &&
           (retries < MAX_WIFI_SETUP_RETRY)) {
        retries++;
        Serial.print(".");
        delay(WIFI_RETRY_DELAY);
    }
    Serial.println();
    return WiFi.status();
}

void setupWebserver() {
    http_rest_server.enableCORS(true);
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html", "Welcome from the REST API\n");
    });
    http_rest_server.on("/time", HTTP_POST, setTimeHandler);
    http_rest_server.on("/time", HTTP_GET, getTimeHandler);
    http_rest_server.on("/coffee", HTTP_GET, getCoffeeCounterHandler);
    Serial.print("Connetted to ");
    Serial.print(WIFI_SSID);
    Serial.print(" --- IP: ");
    Serial.println(WiFi.localIP());
    http_rest_server.begin();
    Serial.println("HTTP REST Server Started");
}

void setTimeHandler() {
    StaticJsonDocument<500> jsonBody;
    String post_body = http_rest_server.arg("plain");
    DeserializationError error = deserializeJson(jsonBody, post_body);
    if (error) {
        Serial.println("error in parsing the json body!");
        http_rest_server.send(400);
    }
    if (jsonBody["mode"] == "Double") {
        int time = jsonBody["time"];
        config.doubleEspressoTime_ms = time;
        Serial.printf("Double Espresso time changed to: %d ms\n", time);
    } else if (jsonBody["mode"] == "Single") {
        int time = jsonBody["time"];
        config.singleEspressoTime_ms = time;
        Serial.printf("Single Espresso time changed to: %d ms\n", time);
    }
    updateConfig(&config);
    http_rest_server.send(200);
}

void getTimeHandler() {
    StaticJsonDocument<200> jsonBody;
    jsonBody["double"] = config.doubleEspressoTime_ms;
    jsonBody["single"] = config.singleEspressoTime_ms;
    String output;
    serializeJson(jsonBody, output);
    http_rest_server.send(200, "application/json", output);
}

void getCoffeeCounterHandler() {
    StaticJsonDocument<200> jsonBody;
    jsonBody["count"] = config.countEspresso;
    String output;
    serializeJson(jsonBody, output);
    http_rest_server.send(200, "application/json", output);
}