
#include "log.h"
#include "sensors.h"

// pump relay gpio
#define PUMP_RELAY_GPIO         4


void water_pump_setup (
        void)
{
    pinMode (PUMP_RELAY_GPIO, OUTPUT);
    digitalWrite (PUMP_RELAY_GPIO, LOW);
}

void water_pump_start (
        void)
{
    DEBUG_LOG_INFO ("water_pump_start");

    digitalWrite (PUMP_RELAY_GPIO, HIGH);
}

void water_pump_stop (
        void)
{
    DEBUG_LOG_INFO ("water_pump_stop");

    digitalWrite (PUMP_RELAY_GPIO, LOW);
}
