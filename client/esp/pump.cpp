#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <string>

#include "defines.hpp"
#include "rpi_gpio.h"
#include "time_utils.h"


// GPIO 4, relay for the water pump
#define SENSOR_RELAY_WATER_PUMP_GPIO        GPIO_NUM_4

// minimum volume that still allows the pamp activation
#define MIN_WATERING_LEVEL                  5


static unsigned long pump_start_time_in_sec;
static unsigned long pump_active_time_in_sec;
static unsigned long pump_stop_time_in_sec;

static f_callback p_callback = NULL;

void do_watering (
        void)
{
    time_t now = time (NULL);
    struct tm tm = *localtime (&now);
    unsigned int curr_time_in_sec = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

    if (curr_time_in_sec < pump_start_time_in_sec || curr_time_in_sec > pump_stop_time_in_sec)
        return;

    int is_enough_water = 1;
    LOG_INFO ("do_watering");

    // get water level before watering
    int liquid_level = get_liquid_level ();
    if (liquid_level < MIN_WATERING_LEVEL * 10)
    {
        is_enough_water = 0;
        LOG_INFO ("water level too low: %d.%d %%, watering canceled",
                liquid_level / 10, liquid_level % 10);
    }
    else
    {
        // start the pump
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);

        // wait enough for the 100 ml
        sleep (pump_active_time_in_sec);

        // stop the pump
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);

        liquid_level = get_liquid_level ();
    }
    int soil_moisture_level = get_soil_moisture_level ();

    // send the message
    if (p_callback != NULL)
    {
        std::vector <std::string> data;
        data.push_back ("\"level\" : " + std::to_string (liquid_level));
        data.push_back ("\"soil\" : " + std::to_string (soil_moisture_level));
        p_callback ("watering", is_enough_water, data);
    }
}

void setup_pump (
        const f_callback f)
{
    p_callback = f;

    // relay
    set_pin_direction (SENSOR_RELAY_WATER_PUMP_GPIO, OUTPUT);

    unsigned long pump_start_time = 800;
    pump_start_time_in_sec = pump_start_time / 100 * 3600 + pump_start_time % 100 * 60;

    pump_active_time_in_sec = 90;
    pump_stop_time_in_sec = pump_start_time_in_sec + pump_active_time_in_sec - 1;

}
