
#include "log.h"
#include "network_manager_class.hpp"
#include "rpi_gpio.h"
#include "rpi_spi.h"
#include "sensor_buzzer.h"
#include "sensor_dht22.h"
#include "settings_class.hpp"
#include "sys_utils.h"

#include <signal.h>
#include <unistd.h>


#define CLIENT_NAME                         "pws"

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

unsigned long dht_last_check_time;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

bool is_time_for_watering (
        void)
{
    time_t now = time (NULL);
    struct tm tm = *localtime (&now);
    unsigned int curr_time_in_sec = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
    unsigned int pump_time = std::stoi (settings_class::get_value_for ("pump_start_time"));
    unsigned int pump_start_time_in_sec = pump_time / 100 * 3600 + pump_time % 100 * 60;
    unsigned int pump_stop_time_in_sec = pump_start_time_in_sec +
            std::stoi (settings_class::get_value_for ("pump_active_time")) - 1;

    //LOG_ERROR ("curr_time_in_sec %d, pump_start_time_in_sec %d", curr_time_in_sec, pump_start_time_in_sec);

    if (curr_time_in_sec >= pump_start_time_in_sec && curr_time_in_sec <= pump_stop_time_in_sec)
        return true;

    return false;
}

bool is_time_for_temperature (
        void)
{
    time_t now = time (NULL);

    //LOG_ERROR ("dht_last_check_time %d, now %ld", dht_last_check_time, now);

    if (now - dht_last_check_time >= (unsigned long)std::stoi (settings_class::get_value_for ("dht_sleep_time")))
    {
        dht_last_check_time = now;
        return true;
    }

    return false;
}


// max 771->796
int get_soil_moisture_level (
        void)
{
    LOG_INFO ("get_soil_moisture_level");

#ifdef USE_WIRINGPI_LIB
    int result = rpi_spi_mcp3008_read (SENSOR_SOIL_MOISTURE_MCP_CHANNEL);
#else
    int result = 666;
#endif // ifdef USE_WIRINGPI_LIB

    LOG_INFO ("soil_moisture_level %d", result);

    return result;
}

int get_liquid_level (
        void)
{
    LOG_INFO ("get_liquid_level");

    // grab the current value from liquide level sensor
#ifdef USE_WIRINGPI_LIB
    int result = rpi_spi_mcp3008_read (SENSOR_LIQUID_LEVEL_MCP_CHANNEL);
#else
    int result = 666;
#endif // ifdef USE_WIRINGPI_LIB

    // fix the range if need (and report about that)
    std::vector <std::string> data;
    if (result > MAX_LIQUID_VALUE)
    {
        data.push_back ("\"level\" : " + std::to_string (result));
        result = MAX_LIQUID_VALUE;
    }
    else if (result < MIN_LIQUID_VALUE)
    {
        data.push_back ("\"level\" : " + std::to_string (result));
        result = MIN_LIQUID_VALUE;
    }

    if (data.size ())
    {
        network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "debug",
                1, data);
    }

    // translate the result to the %  and put decimal part into the value
    result = 100.0 * (MAX_LIQUID_VALUE - result) / (MAX_LIQUID_VALUE - MIN_LIQUID_VALUE) * 10;

    LOG_INFO ("liquid level %d.%d %%", result / 10, result % 10);
    return result;
}

void do_greeting (
        int start_status)
{
    LOG_INFO ("do_greeting");

    // send the message
    std::vector <std::string> data;
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "greeting",
            start_status, data);
}

void do_watering (
        void)
{
    int is_enough_water = 1;
    LOG_INFO ("do_watering");

    // get water level before watering
    int level = get_liquid_level ();
    if (level < MIN_WATERING_LEVEL * 10)
    {
        is_enough_water = 0;
        LOG_INFO ("water level too low: %d.%d %%, watering canceled",
                level / 10, level % 10);
    }
    else
    {
        // start the pump
    #ifdef USE_WIRINGPI_LIB
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);
    #endif // #ifdef USE_WIRINGPI_LIB

        // wait enough for the 100 ml
        sleep (std::stoi (settings_class::get_value_for ("pump_active_time")));

        // stop the pump
    #ifdef USE_WIRINGPI_LIB
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
    #endif // #ifdef USE_WIRINGPI_LIB

        level = get_liquid_level ();
    }

    // send the message
    std::vector <std::string> data;
    data.push_back ("\"level\" : " + std::to_string (level));
    data.push_back ("\"soil\" : " + std::to_string (get_soil_moisture_level ()));
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "watering",
            is_enough_water, data);
}

void do_data_check (
        void)
{
    LOG_INFO ("do_data_check");

    // get the current temperature and humidity
    unsigned int hum; int temp;
#ifdef USE_WIRINGPI_LIB
    int status = dht22_get_data (SENSOR_DHT22_GPIO, &hum, &temp);
#else
    int status = 0; hum = 561; temp = 274;
#endif // ifdef USE_WIRINGPI_LIB

    // send the message
    std::vector <std::string> data;
    if (status == 0)
    {
        //LOG_INFO ("sensor_dht11_get_data %2.1f %%  %2.1f C", hum / 10, temp / 10);
        data.push_back ("\"level\" : " + std::to_string (get_liquid_level ()));
        data.push_back ("\"soil\" : " + std::to_string (get_soil_moisture_level ()));
        data.push_back ("\"hum\" : " + std::to_string (hum));
        data.push_back ("\"temp\" : " + std::to_string (temp));
    }
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "data", status + 1, data);
}

int main (
        void)
{
    signal (SIGINT, exit_function);
    set_app_priority (PRIORITY_MAX);

    // raspberry pi setup
#ifdef USE_WIRINGPI_LIB
    if (geteuid () != 0)
    {
        LOG_ERROR ("need to be root to run");
        is_going_on = 0;
    }

    if (is_going_on)
    {
        if (rpi_gpio_init () == -1)
        {
            LOG_ERROR ("rpi_gpio_init has failed");
            is_going_on = 0;
        }
    }

    if (is_going_on)
    {
        if (rpi_spi_init () == -1)
        {
            LOG_ERROR ("rpi_gpio_init has failed");
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
#endif // ifdef USE_WIRINGPI_LIB

    do_greeting (is_going_on);
    do_data_check ();
    network_manager_class::get_instance ().flush ();

    // start the endless loop
    while (is_going_on)
    {
        if (is_time_for_watering ())
            do_watering ();

        if (is_time_for_temperature ())
            do_data_check ();

        //set_pin_voltage (SENSOR_LED_GPIO, HIGH);
        sleep (1);
        //set_pin_voltage (SENSOR_LED_GPIO, LOW);

        //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_SHORT_BEEP);

        /*int limit;
        int status = get_liquid_level (&limit);
        if (status == 0)
            LOG_INFO ("liquid level %d.%d %%", limit / 10, limit % 10);

        get_soil_moisture_level (&limit);*/
    }

    set_app_priority (PRIORITY_DEFAULT);
    LOG_INFO ("exit app");
    return 0;
}
