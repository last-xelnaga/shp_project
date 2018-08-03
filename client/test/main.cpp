
#include "log.h"
#include "rpi_gpio.h"
#include "rpi_spi.h"
#include "sensor_dht22.h"
#include "sys_utils.h"

#include <signal.h>
#include <unistd.h>

#define CLIENT_NAME                         "swc"

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO                   23

// soil moisture level sensor, connected to mcp3008 channel 0
#define SENSOR_SOIL_MOISTURE_MCP_CHANNEL    0



volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

void do_data_check (
        void)
{
    LOG_INFO ("do_data_check");

    // get the current temperature and humidity
    unsigned int hum; int temp;
    int status = dht22_get_data (SENSOR_DHT22_GPIO, &hum, &temp);
    if (status == 0)
    {
        LOG_INFO ("%d.%d %%    %d.%d C", hum / 10, hum % 10, temp / 10, temp % 10);
    }
}

int main (
        void)
{
    LOG_INFO ("app start");
    //signal (SIGINT, exit_function);
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
#endif // ifdef USE_WIRINGPI_LIB

    while (is_going_on)
    {
        //if (is_time_for_data ())
            do_data_check ();

        //int result = rpi_spi_mcp3008_read (SENSOR_SOIL_MOISTURE_MCP_CHANNEL);
        //LOG_INFO ("level %d", result);

        sleep (2);
    }

    set_app_priority (PRIORITY_DEFAULT);
    LOG_INFO ("exit app");
    return 0;
}
