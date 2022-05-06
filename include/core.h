#include <Arduino.h>
#include <queue>

#define INIT_SINGLE_ESPRESSO_TIME 2000
#define INIT_DOUBLE_ESPRESSO_TIME 4000

typedef struct config_t {
    unsigned singleEspressoTime_ms;
    unsigned doubleEspressoTime_ms;
    unsigned long countEspresso;
    int outputPin;
    int inputPin;
} config_t;

typedef void (*callback_t)(config_t*);
typedef std::queue<callback_t> queue_t;

void initConfig(config_t* config, int inputPin, int outputPin, void (*ISR)());

void taskHandler(config_t* config, queue_t* queue);

void light_sleep();

/**
List of default Tasks

All Task have the callback_t type and recieve the service config.
**/

void coffee_task(config_t* config);