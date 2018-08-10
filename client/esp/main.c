
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>

#include "sensor_dht22.h"
#include "log.h"


// The event group allows multiple bits for each event,
// but we only care about one event - are we connected
// to the AP with an IP?
const int WIFI_CONNECTED_BIT = BIT0;

// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t wifi_event_group;

esp_err_t event_handler (
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

void dht22_task (
        void *pvParameter)
{
    while (true)
    {

        unsigned int humidity;
        int temperature;

        int res = dht22_get_data (GPIO_NUM_4, &humidity, &temperature);
        if (res == 0)
            LOG_INFO ("hum %d.%d %%, temp %d.%d C", humidity / 10, humidity % 10,
                    temperature / 10, temperature % 10);

        vTaskDelay (2000 / portTICK_PERIOD_MS);
    }
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
}

void app_main (
        void)
{
    esp_err_t ret = nvs_flash_init ();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES /*|| ret == ESP_ERR_NVS_NEW_VERSION_FOUND*/)
    {
        ESP_ERROR_CHECK (nvs_flash_erase ());
        ret = nvs_flash_init ();
    }
    ESP_ERROR_CHECK (ret);

    wifi_init_sta ();

    TaskHandle_t xHandle = NULL;
    xTaskCreate (&dht22_task, "dht22_task", 10000, NULL, 1, &xHandle);
    configASSERT (xHandle);

    // Use the handle to delete the task.
    /*if (xHandle != NULL)
    {
        vTaskDelete (xHandle);
    }*/
}
