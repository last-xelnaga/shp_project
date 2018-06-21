
#include "fcm_messaging_class.hpp"
#include "log.h"
#include "server_socket_class.hpp"
#include "server_worker.hpp"

#include <cpp-json/json.h>


typedef struct pws_client_data_t
{
    // client connection info and maintenance
    time_t       client__latest_reg_time;
    unsigned int client__regs_overall;
    time_t       client__lookup_period_start;

    // watering facility
    unsigned int watering__water_level;
    time_t       watering__latest_time;
    unsigned int watering__errors_streak;
    unsigned int watering__errors_overall;

    // temperature and humidity
    int          temp__latest_value;
    int          hum__latest_value;
    time_t       temp__latest_time;
    unsigned int temp__errors_streak;
    unsigned int temp__errors_overall;
} pws_client_data_t;

static pws_client_data_t pws_client_data =
{
    time (NULL), 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 0
};

void show_table (
        void)
{
    fprintf (stderr, "reg_time %ld, watering %ld, temp_last_time %ld, temp %d, hum %d, errors %d                            \r",
            time (NULL) - pws_client_data.client__latest_reg_time, time (NULL) - pws_client_data.watering__latest_time,
            time (NULL) - pws_client_data.temp__latest_time, pws_client_data.temp__latest_value,
            pws_client_data.hum__latest_value, pws_client_data.temp__errors_streak);
    fflush (stderr);
}

int process_message (
        const char* buffer)
{
    int result = 0;
    DEBUG_LOG_INFO ("\n%s", buffer);

    auto json = json::parse (buffer);
    json::value client = json ["client"];
    json::value type = json ["type"];
    std::string type_str = stringify (type, json::ESCAPE_UNICODE);

    json::value status_str = json ["data"]["status"];
    int status = to_number (status_str);

    DEBUG_LOG_INFO ("new %s message", type_str.c_str ());

    if (type_str == "\"dht\"")
    {
        //DEBUG_LOG_INFO ("temp");
        if (status)
        {
            json::value temp = json ["data"]["temp"];
            int temperature = to_number (temp);

            json::value hum = json ["data"]["hum"];
            int humidity = to_number (hum);

            pws_client_data.temp__latest_value = temperature;
            pws_client_data.hum__latest_value = humidity;
            pws_client_data.temp__latest_time = time (NULL);
            pws_client_data.temp__errors_streak = 0;
        }
        else
        {
            pws_client_data.temp__errors_streak ++;
            pws_client_data.temp__errors_overall ++;
        }
    }
    else if (type_str == "\"watering_stop\"")
    {
        //DEBUG_LOG_INFO ("pump");
        //std::string status_str = stringify (status, json::ESCAPE_UNICODE);
        std::string fcm_message_body;
        if (status)
        {
            json::value limit = json ["data"]["limit"];
            pws_client_data.watering__water_level = to_number (limit);

            pws_client_data.watering__latest_time = time (NULL);
            pws_client_data.watering__errors_streak = 0;
            fcm_message_body = "watering successful at\n" + to_string (json ["evt_time"]);
        }
        else
        {
            pws_client_data.watering__errors_streak ++;
            pws_client_data.watering__errors_overall ++;
            fcm_message_body = "failed to do a watering at\n" + to_string (json ["evt_time"]);
        }

        fcm_messaging_class::get_instance ().register_message ("SHP", fcm_message_body.c_str ());
    }
    else if (type_str == "\"app_start\"")
    {
        //DEBUG_LOG_INFO ("start");
        if (status)
        {

        }
        pws_client_data.client__latest_reg_time = time (NULL);
        pws_client_data.client__regs_overall ++;
        pws_client_data.client__lookup_period_start = pws_client_data.client__latest_reg_time - 14 * 24 * 60 * 60;
    }
    else
    {
        //DEBUG_LOG_ERROR ("unknown message");
        result = -1;
    }

    return result;
}

void server_worker (
        server_socket_class::server_client_class* p_client)
{
    unsigned char* buffer = NULL;
    unsigned int size = 0;

    while (p_client->recv_message (&buffer, &size) == 0)
    {
        if (size)
        {
            int result = process_message ((char*)buffer);

            if (result == 0)
                p_client->send_message ((unsigned char*)"OK", 3);
            else
                p_client->send_message ((unsigned char*)"NOK", 4);
        }

        if (buffer)
            delete [] buffer;
        buffer = NULL;
        size = 0;
    }
}
