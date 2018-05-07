
#include "network_manager_class.hpp"
#include "log.h"
#include "sensors.h"
#include "settings_class.hpp"

#include <signal.h>
#include <unistd.h>


volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

static void do_app_start (
        void)
{
    DEBUG_LOG_INFO ("do_app_start");

    std::string message = "  \"type\" : \"app_start\"\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

static void do_watering (
        void)
{
    DEBUG_LOG_INFO ("do_watering");

    // set the type
    std::string message = "  \"type\" : \"watering\",\n";

    // get water level before watering
    int limit = get_liquid_level ();
    message += "  \"data\" : {\n";
    message += "    \"limit\" : " + std::to_string (limit) + ",\n";

    if (limit > 1)
    {
        // start the pump
        water_pump_start ();

        // wait enough for the 100 ml
        sleep (settings_class::get_instance ().get_pump_active_time ());

        // stop the pump
        water_pump_stop ();

        message += "    \"status\" : \"OK\"\n";
    }
    else
    {
        message += "    \"status\" : \"water level too low\"\n";
    }
    message += "  }\n";

    // send the message
    network_manager_class::get_instance ().enqueue_message (message);
}

static void do_temperature_check (
        void)
{
    DEBUG_LOG_INFO ("do_temperature_check");

    // set the type
    std::string message = "  \"type\" : \"dht\",\n";

    // get the current temperature and humidity
    int temp, hum;
    dht_get_data (&temp, &hum);
    message += "  \"data\" : {\n";
    message += "    \"temp\" : " + std::to_string (temp) + ",\n";
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

    if (is_going_on)
        if (board_setup ())
            is_going_on = 0;

    if (is_going_on)
    {
        water_pump_setup ();
        dht_setup ();
        liquid_level_setup ();
    }

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
