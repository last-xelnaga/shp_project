
#include "log.h"
#include "network_manager_class.hpp"
#include "rpi_gpio.h"
#include "sensor_dht22.h"
#include "settings_class.hpp"
#include "sys_utils.h"

#include <signal.h>
#include <unistd.h>

#define CLIENT_NAME                         "swc"

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO                   23


volatile sig_atomic_t is_going_on = 1;

static unsigned long data_last_check_time_in_sec;
static unsigned long data_check_sleep_time_in_sec;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

bool is_time_for_data (
        void)
{
    bool result = false;

    time_t now = time (NULL);

    if (now - data_last_check_time_in_sec >= data_check_sleep_time_in_sec)
    {
        data_last_check_time_in_sec = now;
        result = true;
    }

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
        data.push_back ("\"hum\" : " + std::to_string (hum));
        data.push_back ("\"temp\" : " + std::to_string (temp));
    }
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "data", status + 1, data);
}

int main (
        void)
{
    LOG_INFO ("app start");
    signal (SIGINT, exit_function);
    set_app_priority (PRIORITY_MAX);

    data_check_sleep_time_in_sec = std::stoi (settings_class::get_value_for ("data_check_sleep_time"));

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
#endif // ifdef USE_WIRINGPI_LIB

    network_manager_class::get_instance ().set_server_address (
            settings_class::get_value_for ("server_name"),
            std::stoi (settings_class::get_value_for ("server_port")));

    do_greeting (is_going_on);
    do_data_check ();
    network_manager_class::get_instance ().flush ();

    while (is_going_on)
    {
        if (is_time_for_data ())
            do_data_check ();

        sleep (1);
    }

    set_app_priority (PRIORITY_DEFAULT);
    LOG_INFO ("exit app");
    return 0;
}
