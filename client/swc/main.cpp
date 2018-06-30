
#include "log.h"
#include "rpi_gpio.h"
#include "rpi_spi.h"
#include "sensor_buzzer.h"
#include "sensor_dht22.h"

#include <signal.h>
#include <unistd.h>

// pump relay gpio
#define SENSOR_RELAY_WATER_PUMP_GPIO    18 //4

// dht22 temperature and humidity sensor gpio
#define SENSOR_DHT22_GPIO               4 //6

// dht22 temperature and humidity sensor gpio
#define SENSOR_BUZZER_GPIO              23


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
    LOG_INFO ("liquid_level_setup");
}

int get_liquid_level (
        int* limit)
{
    LOG_INFO ("get_liquid_level");

    *limit = 55;

    /*char send_data [] = {0x01, 0x80, 0};
    char buf [] = {0, 0, 0, 0, 0};

    bcm2835_spi_transfernb (send_data, buf, sizeof (send_data));

    int a2dVal = (buf[1]<< 8) & 0b1100000000;
    a2dVal |=  (buf[2] & 0xff);
    printf ("val %d, %d mm\n", a2dVal, int (.7095 * (752 - a2dVal)));*/

    return 0;
}

int main (
        void)
{
    LOG_INFO ("app start");
    signal (SIGINT, exit_function);

    if (geteuid () != 0)
    {
        LOG_ERROR ("need to be root to run");
        is_going_on = 0;
    }

    // raspberry pi setup
    #ifdef USE_WIRINGPI_LIB
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

        liquid_level_setup ();
    }
    #endif // #ifdef USE_WIRINGPI_LIB

    int count = 0;
    while (is_going_on)
    {
        if (count == 0)
        {
            // start the pump
            #ifdef USE_WIRINGPI_LIB
            set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);
            #endif // #ifdef USE_WIRINGPI_LIB

            // get the current temperature and humidity
            unsigned int hum; int temp;
            int status = dht22_get_data (SENSOR_DHT22_GPIO, &hum, &temp);
            if (status == 0)
                LOG_INFO ("sensor_dht11_get_data %2.1f %%   %2.1f C", (float)hum / 10, (float)temp / 10);

            sleep (1);

            // stop the pump
            #ifdef USE_WIRINGPI_LIB
            set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
            #endif // #ifdef USE_WIRINGPI_LIB


            buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_SHORT_BEEP);
            //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_LONG_BEEP);
            //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_TWO_SHORT_BEEPS);
        }

        //get_liquid_level ();
        sleep (1);

        count ++;
        if (count == 5)
            count = 0;
    }

    LOG_INFO ("app exit");
    return 0;
}
