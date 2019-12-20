#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>

#include <string>

#include "defines.hpp"
#include "rpi_gpio.h"
#include "sensor_dht22.h"
#include "time_utils.h"


// how often check for temperature, humidity,
// water level and soil moisture. in secs
#define SENSOR_CHECK_PERIOD     30


// GPIO 4, dht22, temperature & humidity
#define DHT_PIN                 GPIO_NUM_17


// GPIO 39, ADC 3, soil moisture
#define MOISTURE_ADC            ADC1_CHANNEL_3

// value range for the soil moisture sensor
#define MOISTURE_VALUE_MIN      390
#define MOISTURE_VALUE_MAX      811


// GPIO 36, ADC 0, liquid level
#define LEVEL_ADC               ADC1_CHANNEL_0

// value range for liquid level sensor
#define WATER_LEVEL_MIN         619
#define WATER_LEVEL_MAX         790


static portMUX_TYPE adc_access_mitex = portMUX_INITIALIZER_UNLOCKED;

static f_callback p_callback = NULL;

static void gpio_task_worker (
        void* arg)
{
    for (;;)
    {
        LOG_INFO ("do_data_check");

        // get the current temperature and humidity
        unsigned int humidity;
        int temperature;

        int res = dht22_get_data (DHT_PIN, &humidity, &temperature);
        if (res != 0)
        {
            sleep_milliseconds (2000);
            res = dht22_get_data (DHT_PIN, &humidity, &temperature);
        }

        //LOG_INFO ("hum %d.%d %%, temp %d.%d C", humidity / 10, humidity % 10,
        //        temperature / 10, temperature % 10);

        int soil_moisture_level = get_soil_moisture_level ();
        int liquid_level = get_liquid_level ();

        // send the message
        if (p_callback != NULL)
        {
            std::vector <std::string> data;
            data.push_back ("\"level\" : " + std::to_string (liquid_level));
            data.push_back ("\"soil\" : " + std::to_string (soil_moisture_level));
            data.push_back ("\"hum\" : " + std::to_string (humidity));
            data.push_back ("\"temp\" : " + std::to_string (temperature));
            p_callback ("sensors", res, data);
        }

        sleep_milliseconds (SENSOR_CHECK_PERIOD * 1000);
    }
}

int get_soil_moisture_level (
        void)
{
    // enter critical section for the adc access,
    // grab the value and make the adc avalable once again
    portENTER_CRITICAL (&adc_access_mitex);
    int result_raw = adc1_get_raw (MOISTURE_ADC);
    portEXIT_CRITICAL (&adc_access_mitex);

    // adjust the value to be sure that it will fit
    int result = result_raw;
    if (result_raw > MOISTURE_VALUE_MAX)
        result = MOISTURE_VALUE_MAX;
    if (result_raw < MOISTURE_VALUE_MIN)
        result = MOISTURE_VALUE_MIN;

    // let's convert the value to %%
    result = (result - MOISTURE_VALUE_MIN) * 1000 / (MOISTURE_VALUE_MAX - MOISTURE_VALUE_MIN);
    result = 1000 - result;

    //LOG_INFO ("soil_moisture_level %d,%d %%, (%d)", result / 10, result % 10, result_raw);
    return result;
}

int get_liquid_level (
        void)
{
    // enter critical section for the adc access,
    // grab the value and make the adc avalable once again
    portENTER_CRITICAL (&adc_access_mitex);
    int result_raw = adc1_get_raw (LEVEL_ADC);
    portEXIT_CRITICAL (&adc_access_mitex);

    // adjust the value to be sure that it will fit
    int result = result_raw;
    if (result_raw > WATER_LEVEL_MAX)
        result = WATER_LEVEL_MAX;
    if (result_raw < WATER_LEVEL_MIN)
        result = WATER_LEVEL_MIN;

    result = (result - WATER_LEVEL_MIN) * 1000 / (WATER_LEVEL_MAX - WATER_LEVEL_MIN);
    result = 1000 - result;

    //LOG_INFO ("liquid_level %d,%d %%, (%d)", result / 10, result % 10, result_raw);
    return result;
}

// init sensors and start the task
void setup_sensors (
        const f_callback f)
{
    p_callback = f;

    // adc inputs
    adc1_config_width (ADC_WIDTH_BIT_10);
    adc1_config_channel_atten (LEVEL_ADC, ADC_ATTEN_11db);
    adc1_config_channel_atten (MOISTURE_ADC, ADC_ATTEN_11db);

    // dht
    set_pin_direction (DHT_PIN, OUTPUT);

    xTaskCreate (&gpio_task_worker, "sensor_task", 4096, NULL, 5, NULL);
}
