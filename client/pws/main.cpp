
#include "network_manager_class.hpp"
#include "log.h"
#include "rpi_gpio.h"
#include "rpi_spi.h"
#include "sensor_buzzer.h"
#include "sensor_dht22.h"
#include "settings_class.hpp"

#include <signal.h>
#include <unistd.h>


// pump relay gpio
#define SENSOR_RELAY_WATER_PUMP_GPIO        4

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO                   6

// led gpio
#define SENSOR_LED_GPIO                     17

// buzzer gpio
#define SENSOR_BUZZER_GPIO                  18

// eType liquide level sensor, connected to mcp3008 channel 0
#define SENSOR_LIQUID_LEVEL_MCP_CHANNEL     0

// sensor value for full water tank
#define MIN_LIQUID_VALUE                    619

// sensor value for empty water tank
#define MAX_LIQUID_VALUE                    790

// minimum volume that still allows the pamp activation
#define MIN_WATERING_LEVEL                  2

// eType liquide level sensor, connected to mcp3008 channel 1
#define SENSOR_SOIL_MOISTURE_MCP_CHANNEL    1

volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

// max 771->796
int get_soil_moisture_level (
        void)
{
    int result = rpi_spi_mcp3008_read (SENSOR_SOIL_MOISTURE_MCP_CHANNEL);
    DEBUG_LOG_INFO ("soil_moisture_level %d", result);

    return result;
}

int get_liquid_level (
        int* level)
{
    int status = -1;
    DEBUG_LOG_INFO ("get_liquid_level");

    int result = rpi_spi_mcp3008_read (SENSOR_LIQUID_LEVEL_MCP_CHANNEL);

    if (result > MAX_LIQUID_VALUE)
        result = MAX_LIQUID_VALUE;
    if (result < MIN_LIQUID_VALUE)
        result = MIN_LIQUID_VALUE;

    float x = 1000.0 * (MAX_LIQUID_VALUE - result) / (MAX_LIQUID_VALUE - MIN_LIQUID_VALUE);
    if (x > MIN_WATERING_LEVEL * 10)
    {
        status = 0;
        *level = (int)x;
    }

    DEBUG_LOG_INFO ("liquid level %d.%d %%", *level / 10, *level % 10);

    return status;
}

void do_app_start (
        int start_status)
{
    DEBUG_LOG_INFO ("do_app_start");

    std::string message = "  \"type\" : \"app_start\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (start_status) + "\n";
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

void do_watering (
        void)
{
    DEBUG_LOG_INFO ("do_watering");

    // get water level before watering
    int level;
    int status = get_liquid_level (&level);

    // set the type
    std::string message = "  \"type\" : \"watering_start\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (status + 1) + ",\n";
    message += "    \"soil\" : " + std::to_string (get_soil_moisture_level ());
    if (status == 0)
    {
        message += ",\n";
        message += "    \"level\" : " + std::to_string (level) + "\n";
    }
    else
    {
        message += "\n";
    }
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);


    if (status != 0)
        return;

    // start the pump
    #ifdef USE_WIRINGPI_LIB
    set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);
    #endif // #ifdef USE_WIRINGPI_LIB

    // wait enough for the 100 ml
    sleep (settings_class::get_instance ().get_pump_active_time ());

    // stop the pump
    #ifdef USE_WIRINGPI_LIB
    set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
    #endif // #ifdef USE_WIRINGPI_LIB


    status = get_liquid_level (&level);

    // set the type
    message = "  \"type\" : \"watering_stop\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (status + 1) + ",\n";
    message += "    \"soil\" : " + std::to_string (get_soil_moisture_level ());
    if (status == 0)
    {
        message += ",\n";
        message += "    \"level\" : " + std::to_string (level) + "\n";
    }
    else
    {
        message += "\n";
    }
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

void do_temperature_check (
        void)
{
    DEBUG_LOG_INFO ("do_temperature_check");

    // get the current temperature and humidity
    unsigned int hum; int temp;
    int status = dht22_get_data (SENSOR_DHT22_GPIO, &hum, &temp);
    if (status == 0)
        DEBUG_LOG_INFO ("sensor_dht11_get_data %2.1f %%   %2.1f C", (float)hum / 10, (float)temp / 10);

    // set the type
    std::string message = "  \"type\" : \"dht\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (status + 1);
    if (status == 0)
    {
        message += ",\n";
        message += "    \"hum\" : " + std::to_string (hum) + ",\n";
        message += "    \"temp\" : " + std::to_string (temp) + "\n";
    }
    else
    {
        message += "\n";
    }
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

int main (
        void)
{
    signal (SIGINT, exit_function);

    if (geteuid () != 0)
    {
        DEBUG_LOG_ERROR ("need to be root to run");
        is_going_on = 0;
    }

    // raspberry pi setup
    #ifdef USE_WIRINGPI_LIB
    if (is_going_on)
    {
        if (rpi_gpio_init () == -1)
        {
            DEBUG_LOG_ERROR ("rpi_gpio_init has failed");
            is_going_on = 0;
        }
    }

    if (is_going_on)
    {
        if (rpi_spi_init () == -1)
        {
            DEBUG_LOG_ERROR ("rpi_gpio_init has failed");
            is_going_on = 0;
        }
    }

    if (is_going_on)
    {
        set_pin_direction (SENSOR_RELAY_WATER_PUMP_GPIO, OUTPUT);
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);

        set_pin_direction (SENSOR_LED_GPIO, OUTPUT);
        set_pin_voltage (SENSOR_LED_GPIO, LOW);
    }

    //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_SHORT_BEEP);
    //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_LONG_BEEP);
    buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_TWO_SHORT_BEEPS);
    #endif // #ifdef USE_WIRINGPI_LIB

    do_app_start (is_going_on);

    while (is_going_on)
    {
        if (settings_class::get_instance ().is_time_for_watering ())
            do_watering ();

        if (settings_class::get_instance ().is_time_for_temperature ())
        {
            do_temperature_check ();

            int limit;
            get_liquid_level (&limit);
            get_soil_moisture_level ();
        }

        //set_pin_voltage (SENSOR_LED_GPIO, HIGH);
        sleep (1);
        //set_pin_voltage (SENSOR_LED_GPIO, LOW);

        //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_SHORT_BEEP);

        /*int limit;
        int status = get_liquid_level (&limit);
        if (status == 0)
            DEBUG_LOG_INFO ("liquid level %d.%d %%", limit / 10, limit % 10);

        get_soil_moisture_level (&limit);*/
    }

    DEBUG_LOG_INFO ("exit app");

    return 0;
}
