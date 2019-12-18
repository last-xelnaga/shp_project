

//#include <esp_event_loop.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>

#include "defines.hpp"
#include "time_utils.h"


extern "C"
{
    void app_main (void);
}


// The event group allows multiple bits for each event,
// but we only care about one event - are we connected
// to the AP with an IP?
const int WIFI_CONNECTED_BIT = BIT0;


// FreeRTOS event group to signal when we are connected
/*static EventGroupHandle_t wifi_event_group;

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



void func (
        std::string type,
        const int status,
        std::vector <std::string> data)
{
    std::string evt_message = "{\n";
    evt_message += "  \"client\" : \"esp\",\n";
    evt_message += "  \"evt_time\" : \"" + std::string (get_time_str ()) + "\",\n";
    evt_message += "  \"evt_time_unix\" : " + std::to_string (time (NULL)) + ",\n";
    evt_message += "  \"type\" : \"" + type + "\",\n";
    evt_message += "  \"data\" : {\n";

    int size = data.size ();
    for (int i = 0; i < size; ++ i)
        evt_message += "    " + std::string (data [i]) + ",\n";

    evt_message += "    \"status\" : " + std::to_string (status) + "\n";
    evt_message += "  }\n";
    evt_message += "}\n";

    LOG_INFO ("%s", evt_message.c_str ());
}

void app_main (
        void)
{
    //init_storage ();
    //wifi_init_sta ();

    // do greeting
    std::vector <std::string> data;
    func ("greeting", 1, data);

    setup_sensors (func);
    setup_button (func);
    setup_pump (func);

    // start the endless loop
    while (1)
    {
        //do_watering ();

        //LOG_INFO ("(main) soil %d", get_soil_moisture_level ());
        do_watering ();
        sleep_milliseconds (1000);
    }
}
