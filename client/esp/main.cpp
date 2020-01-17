
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
extern "C"
{
    #include "esp_wifi.h"
}
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_data.h"
#include "defines.hpp"
#include "time_utils.h"


// The event group allows multiple bits for each event,
// but we only care about one event - are we connected
// to the AP with an IP?
const int WIFI_CONNECTED_BIT = BIT0;

// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t wifi_event_group;

extern "C"
{
    void app_main (void);
}

static void event_handler (
        void* arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            LOG_INFO ("STA event: WIFI_EVENT_STA_START");
            esp_wifi_connect ();
        }
        else if (event_id == WIFI_EVENT_STA_CONNECTED)
        {
            LOG_INFO ("STA event: WIFI_EVENT_STA_CONNECTED");
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            xEventGroupClearBits (wifi_event_group, WIFI_CONNECTED_BIT);
            LOG_INFO ("STA event: WIFI_EVENT_STA_DISCONNECTED");

            sleep_milliseconds (3000);
            LOG_INFO ("STA event: try to re-connect");
            esp_wifi_connect ();

        }
        else
        {
            LOG_INFO ("STA event: unknown WIFI_EVENT %d", event_id);
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            xEventGroupSetBits (wifi_event_group, WIFI_CONNECTED_BIT);
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            LOG_INFO ("STA event IP_EVENT_STA_GOT_IP ip: " IPSTR, IP2STR (&event->ip_info.ip));
        }
        else
        {
            LOG_INFO ("STA event: unknown IP_EVENT %d", event_id);
        }
    }
    else
    {
        LOG_INFO ("STA event: unknown even %d base, %d id", event_base, event_id);
    }
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
    ESP_ERROR_CHECK (esp_netif_init ());
    ESP_ERROR_CHECK (esp_event_loop_create_default ());

    //esp_netif_t *sta_netif =
    esp_netif_create_default_wifi_sta ();
    //assert (sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
    ESP_ERROR_CHECK (esp_wifi_init (&cfg));

    wifi_config_t wifi_config;
    bzero (&wifi_config, sizeof (wifi_config_t));
    strcpy ((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy ((char*)wifi_config.sta.password, WIFI_PASSWORD);
    ESP_ERROR_CHECK (esp_wifi_set_config (ESP_IF_WIFI_STA, &wifi_config));

    ESP_ERROR_CHECK (esp_event_handler_register (WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK (esp_event_handler_register (IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));
    //ESP_ERROR_CHECK (esp_wifi_set_protocol (WIFI_PROTOCOL_11N));
    ESP_ERROR_CHECK (esp_wifi_start ());
}



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

    LOG_INFO ("\n%s", evt_message.c_str ());
}

void app_main (
        void)
{
    wifi_event_group = xEventGroupCreate ();

    init_storage ();
    wifi_init_sta ();

    EventBits_t uxBits;
    while (1)
    {
        uxBits = xEventGroupWaitBits (wifi_event_group, WIFI_CONNECTED_BIT, true, false, portMAX_DELAY);
        if (uxBits & WIFI_CONNECTED_BIT)
        {
            LOG_INFO ("WiFi Connected to ap");
            break;
        }
    }

    // do greeting
    std::vector <std::string> data;
    func ("greeting", 0, data);

    setup_sensors (func);
    setup_button (func);
    setup_pump (func);

    // start the endless loop
    while (1)
    {
        do_watering ();

        //LOG_INFO ("(main) soil %d", get_soil_moisture_level ());
        //do_watering ();
        sleep_milliseconds (1000);
    }
}
