
#include "network_manager_class.hpp"
#include "log.h"
#include "sensors.h"
#include "settings_class.hpp"

#include <signal.h>
#include <string>
#include <time.h>
#include <unistd.h>


volatile sig_atomic_t exit_flag = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    exit_flag = 0;
}

void do_app_start (
        void)
{
    DEBUG_LOG_INFO ("do_app_start");

    std::string message = "  \"app_start\" : null\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

void do_watering (
        void)
{
    DEBUG_LOG_INFO ("do_watering");

    std::string message = "  \"watering\" : {\n";

    // get water level before watering
    int limit = get_liquid_level ();
    message += "    \"limit\" : " + std::to_string (limit) + ",\n";

    if (limit > 1)
    {
        // start the pump
        if (water_pump_start () == 0)
        {
            // wait enough for the 100 ml
            sleep (settings_class::get_instance ().get_pump_active_time ());

            // stop the pump
            water_pump_stop ();

            message += "    \"status\" : \"OK\"\n";
        }
        else
        {
            message += "    \"status\" : \"failed to start pump\"\n";
        }
    }
    else
    {
        message += "    \"status\" : \"water level too low\"\n";
    }
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

void do_temperature_check (
        void)
{
    DEBUG_LOG_INFO ("do_temperature_check");

    std::string message = "  \"dth\" : {\n";

    // get the current temperature
    int temp = get_temperature ();
    message += "    \"temp\" : " + std::to_string (temp) + ",\n";

    // and humidity
    int hum = get_humidity ();
    message += "    \"hum\" : " + std::to_string (hum) + "\n";

    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

int main (
        void)
{
    signal (SIGINT, exit_function);

    do_app_start ();

    while (exit_flag)
    {
        if (settings_class::get_instance ().is_time_for_watering ())
        {
            do_watering ();
        }

        if (settings_class::get_instance ().is_time_for_temperature ())
        {
            do_temperature_check ();
        }

        sleep (1);
    }

    DEBUG_LOG_INFO ("exit app");

    return 0;
}
