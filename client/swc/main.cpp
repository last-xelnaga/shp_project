
#include "log.h"
#include "rpi_gpio.h"
#include "sensor_dht22.h"
#include "sys_utils.h"

#include <signal.h>
#include <unistd.h>

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO               23


volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

int main (
        void)
{
    LOG_INFO ("app start");
    set_app_priority (PRIORITY_MAX);
    signal (SIGINT, exit_function);

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

    int count = 0;
    while (is_going_on)
    {
        if (count == 0)
        {
            // get the current temperature and humidity
            unsigned int hum; int temp;
            int status = dht22_get_data (SENSOR_DHT22_GPIO, &hum, &temp);
            if (status == 0)
                LOG_INFO ("sensor_dht11_get_data %2.1f %%   %2.1f C", (float)hum / 10, (float)temp / 10);
        }

        count ++;
        if (count == 5)
            count = 0;

        sleep (1);
    }

    set_app_priority (PRIORITY_DEFAULT);
    LOG_INFO ("app exit");
    return 0;
}
