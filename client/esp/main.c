
#include <driver/adc.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>

//#include <string>

#include "log.h"
#include "rpi_gpio.h"
#include "sensor_dht22.h"
#include "time_utils.h"


volatile char is_button_pressed = 0;

// The event group allows multiple bits for each event,
// but we only care about one event - are we connected
// to the AP with an IP?
const int WIFI_CONNECTED_BIT = BIT0;

// FreeRTOS event group to signal when we are connected
//static EventGroupHandle_t wifi_event_group;

/*esp_err_t event_handler (
        void* ctx,
        system_event_t* event)
{
    switch (event->event_id)
    {
        case SYSTEM_EVENT_STA_START:
            LOG_INFO ("wifi event SYSTEM_EVENT_STA_START");
            esp_wifi_connect ();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            LOG_INFO ("wifi event SYSTEM_EVENT_STA_GOT_IP %s",
                    ip4addr_ntoa (&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits (wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            LOG_INFO ("wifi event SYSTEM_EVENT_AP_STACONNECTED AID=%d",
                    event->event_info.sta_connected.aid);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            LOG_INFO ("wifi event SYSTEM_EVENT_AP_STADISCONNECTED AID=%d",
                    event->event_info.sta_disconnected.aid);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            LOG_INFO ("wifi event SYSTEM_EVENT_STA_DISCONNECTED");
            esp_wifi_connect ();
            xEventGroupClearBits (wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }

    return ESP_OK;
}

void wifi_init_sta (
        void)
{
    wifi_event_group = xEventGroupCreate ();

    tcpip_adapter_init ();
    ESP_ERROR_CHECK (esp_event_loop_init (event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
    ESP_ERROR_CHECK (esp_wifi_init (&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ssid",
            .password = "pass"
        },
    };

    ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));
    ESP_ERROR_CHECK (esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK (esp_wifi_start ());

    LOG_INFO ("wifi_init_sta finished.");
}*/


// GPIO 36, ADC 0, liquid level
#define LEVEL_ADC       ADC1_CHANNEL_0
// GPIO 39, ADC 3, soil moisture
#define MOISTURE_ADC    ADC1_CHANNEL_3

// GPIO 32, 33, 25, 26, 27, leds
#define LED1_PIN        GPIO_NUM_32
#define LED2_PIN        GPIO_NUM_33
#define LED3_PIN        GPIO_NUM_25
#define LED4_PIN        GPIO_NUM_26
#define LED5_PIN        GPIO_NUM_27

// GPIO 4, relay for the water pump
#define SENSOR_RELAY_WATER_PUMP_GPIO       GPIO_NUM_4

// GPIO 27, button
#define BUTTON_PIN      GPIO_NUM_16

// GPIO 4, dht22, temperature & humidity
#define DHT_PIN         GPIO_NUM_17


// min sensor value for full water tank
#define MIN_LIQUID_VALUE                    619

// max sensor value for empty water tank
#define MAX_LIQUID_VALUE                    790

// minimum volume that still allows the pamp activation
#define MIN_WATERING_LEVEL                  2


static unsigned long pump_start_time_in_sec;
static unsigned long pump_active_time_in_sec;
static unsigned long pump_stop_time_in_sec;

static unsigned long data_last_check_time_in_sec;
static unsigned long data_check_sleep_time_in_sec;


void init_storage (
        void)
{
    esp_err_t ret = nvs_flash_init ();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK (nvs_flash_erase ());
        ret = nvs_flash_init ();
    }
    ESP_ERROR_CHECK (ret);
}

void button_task (
        void* pvParameter)
{
    while (1)
    {
        while (get_bus_state (BUTTON_PIN) == LOW)
            sleep_milliseconds (50);

        LOG_INFO ("button pressed");
        is_button_pressed = 1;

        while (get_bus_state (BUTTON_PIN) == HIGH)
            sleep_milliseconds (50);
    }
}

int get_soil_moisture_level (
        void)
{
    int result = adc1_get_raw (MOISTURE_ADC);
    LOG_INFO ("soil_moisture_level %d", result);
    return result;
}

int get_liquid_level (
        void)
{
    int result = adc1_get_raw (LEVEL_ADC);
    LOG_INFO ("get_liquid_level %d", result);
    return result;
}

void do_greeting (
        int start_status)
{
    LOG_INFO ("do_greeting");

    // send the message
    /*std::vector <std::string> data;
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "greeting",
            start_status, data);*/
}

void do_watering (
        void)
{
    time_t now = time (NULL);
    struct tm tm = *localtime (&now);
    unsigned int curr_time_in_sec = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

    if (curr_time_in_sec < pump_start_time_in_sec || curr_time_in_sec > pump_stop_time_in_sec)
        return;

    //int is_enough_water = 1;
    LOG_INFO ("do_watering");

    //buzzer_play_sound (SENSOR_BUZZER_GPIO, BUZZER_SHORT_BEEP);

    // get water level before watering
    int level = get_liquid_level ();
    if (level < MIN_WATERING_LEVEL * 10)
    {
        //is_enough_water = 0;
        LOG_INFO ("water level too low: %d.%d %%, watering canceled",
                level / 10, level % 10);
    }
    else
    {
        // start the pump
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);

        // wait enough for the 100 ml
        sleep (pump_active_time_in_sec);

        // stop the pump
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);

        level = get_liquid_level ();
    }

    // send the message
    /*std::vector <std::string> data;
    data.push_back ("\"level\" : " + std::to_string (level));
    data.push_back ("\"soil\" : " + std::to_string (get_soil_moisture_level ()));
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "watering",
            is_enough_water, data);*/
}

void do_data_check (
        void)
{
    time_t now = time (NULL);

    if (now - data_last_check_time_in_sec >= data_check_sleep_time_in_sec)
        data_last_check_time_in_sec = now;
    else
        return;

    LOG_INFO ("do_data_check");

    // get the current temperature and humidity
    unsigned int humidity;
    int temperature;

    int res = dht22_get_data (DHT_PIN, &humidity, &temperature);
    if (res == 0)
        LOG_INFO ("hum %d.%d %%, temp %d.%d C", humidity / 10, humidity % 10,
                temperature / 10, temperature % 10);

    // send the message
    /*std::vector <std::string> data;
    if (status == 0)
    {
        //LOG_INFO ("sensor_dht11_get_data %2.1f %%  %2.1f C", hum / 10, temp / 10);
        data.push_back ("\"level\" : " + std::to_string (get_liquid_level ()));
        data.push_back ("\"soil\" : " + std::to_string (get_soil_moisture_level ()));
        data.push_back ("\"hum\" : " + std::to_string (hum));
        data.push_back ("\"temp\" : " + std::to_string (temp));
    }
    network_manager_class::get_instance ().enqueue_message (CLIENT_NAME, "data", status + 1, data);*/
}

void watering_task (
        void* pvParameter)
{
    while (1)
    {
        if (!is_button_pressed)
        {
            sleep_milliseconds (50);
            continue;
        }

        LOG_INFO ("watering_task on");
        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);

        bool toggle = true;
        time_t task_curr_time = time (NULL);
        time_t task_stop_time = task_curr_time + 100;
        while (task_stop_time > task_curr_time)
        {
            int val = adc1_get_raw (LEVEL_ADC);
            int level = ((float) val / 10.24);
            LOG_INFO ("level val %d, it's %d %%", val, level);

            if (toggle)
            {
                set_pin_voltage (LED1_PIN, HIGH);
                set_pin_voltage (LED2_PIN, HIGH);
                set_pin_voltage (LED3_PIN, HIGH);
                set_pin_voltage (LED4_PIN, HIGH);
                set_pin_voltage (LED5_PIN, HIGH);
                toggle = false;
            }
            else
            {
                if (level < 95)
                    set_pin_voltage (LED5_PIN, LOW);
                if (level < 80)
                    set_pin_voltage (LED4_PIN, LOW);
                if (level < 60)
                    set_pin_voltage (LED3_PIN, LOW);
                if (level < 40)
                    set_pin_voltage (LED2_PIN, LOW);
                if (level < 20)
                    set_pin_voltage (LED1_PIN, LOW);

                toggle = true;
            }

            sleep_milliseconds (300);
            task_curr_time = time (NULL);
        }

        set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
        LOG_INFO ("watering_task off");
        is_button_pressed = 0;
    }
}

void app_main (
        void)
{
    unsigned long pump_start_time = 800;
    pump_start_time_in_sec = pump_start_time / 100 * 3600 + pump_start_time % 100 * 60;

    pump_active_time_in_sec = 90;
    pump_stop_time_in_sec = pump_start_time_in_sec + pump_active_time_in_sec - 1;

    data_check_sleep_time_in_sec = 10;

    //init_storage ();
    //wifi_init_sta ();

    // adc inputs
    adc1_config_width (ADC_WIDTH_BIT_10);
    adc1_config_channel_atten (LEVEL_ADC, ADC_ATTEN_11db);
    adc1_config_channel_atten (MOISTURE_ADC, ADC_ATTEN_11db);

    // leds
    set_pin_direction (LED1_PIN, OUTPUT);
    set_pin_direction (LED2_PIN, OUTPUT);
    set_pin_direction (LED3_PIN, OUTPUT);
    set_pin_direction (LED4_PIN, OUTPUT);
    set_pin_direction (LED5_PIN, OUTPUT);

    // relay
    set_pin_direction (SENSOR_RELAY_WATER_PUMP_GPIO, OUTPUT);

    // button
    set_pin_direction (BUTTON_PIN, INPUT);

    // dht
    set_pin_direction (DHT_PIN, OUTPUT);

    xTaskCreate (&button_task, "button_task", 2000, NULL, 5, NULL);
    //xTaskCreate (&watering_task, "watering_task", 10000, NULL, 5, NULL);
    //xTaskCreate (&dht22_task, "dht22_task", 10000, NULL, 5, NULL);

    /*while (1)
    {
        int val = adc1_get_raw (MOISTURE_ADC);
        int level = ((float) val / 10.24);
        LOG_INFO ("dry val %d, it's %d %%", val, level);

        sleep (1);
    }*/

    // start the endless loop
    while (1)
    {
        do_watering ();
        do_data_check ();


        sleep_milliseconds (1000);
    }
}
