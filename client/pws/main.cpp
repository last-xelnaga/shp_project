
#include "network_manager_class.hpp"
#include "log.h"
#include "rpi_gpio.h"
#include "rpi_spi.h"
#include "sensor_dht22.h"
#include "settings_class.hpp"

#include <signal.h>
#include <unistd.h>


// pump relay gpio
#define SENSOR_RELAY_WATER_PUMP_GPIO    4

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO               6


volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

// liquid_level sensor
void liquid_level_setup (
        void)
{
    DEBUG_LOG_INFO ("liquid_level_setup");
}

int get_liquid_level (
        int* limit)
{
    DEBUG_LOG_INFO ("get_liquid_level");

    *limit = 55;

    char send_data [] = {0x01, 0x80, 0};
    char buf [] = {0, 0, 0, 0, 0};

    bcm2835_spi_transfernb (send_data, buf, sizeof (send_data));

    int a2dVal = (buf[1]<< 8) & 0b1100000000;
    a2dVal |=  (buf[2] & 0xff);
    printf ("val %d, %d mm\n", a2dVal, int (.7095 * (752 - a2dVal)));

    return 0;
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
    int limit;
    int status = get_liquid_level (&limit);

    // set the type
    std::string message = "  \"type\" : \"watering_start\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (status + 1);
    if (status == 0)
    {
        message += ",\n";
        message += "    \"limit\" : " + std::to_string (limit) + "\n";
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


    status = get_liquid_level (&limit);

    // set the type
    message = "  \"type\" : \"watering_stop\",\n";
    message += "  \"data\" : {\n";
    message += "    \"status\" : " + std::to_string (status + 1);
    if (status == 0)
    {
        message += ",\n";
        message += "    \"limit\" : " + std::to_string (limit) + "\n";
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

        liquid_level_setup ();
    }
    #endif // #ifdef USE_WIRINGPI_LIB

    do_app_start (is_going_on);

    while (is_going_on)
    {
        if (settings_class::get_instance ().is_time_for_watering ())
            do_watering ();

        if (settings_class::get_instance ().is_time_for_temperature ())
            do_temperature_check ();

        sleep (1);
    }

    DEBUG_LOG_INFO ("exit app");

    return 0;
}
