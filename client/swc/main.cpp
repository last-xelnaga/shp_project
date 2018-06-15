
//#include "client_socket_class.hpp"
#include "log.h"
#include "sensors.h"
#include "sensor_dht22.h"


#include <string>
#include <time.h>
#include <signal.h>
#include <unistd.h>


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
    DEBUG_LOG_INFO ("app start");
    signal (SIGINT, exit_function);

    /*while (is_going_on)
    {
        DEBUG_LOG_INFO ("new try...");

        time_t now = time (NULL);

        std::string evt_message = "{\n";
        evt_message += "  \"client\" : \"pws\",\n";
        //evt_message += "  \"evt_time\" : \"" + std::string (get_time_str (now)) + "\",\n";
        evt_message += "  \"evt_time_unix\" : " + std::to_string (now) + ",\n";
        evt_message += "  \"type\" : \"app_start\"\n";
        evt_message += "}\n";

        client_socket_class socket_client;
        int status = socket_client.connect ("localhost",
                5000);
        if (status == 0)
        {
            unsigned char* p_answer = NULL;
            unsigned int answer_size = 0;
            status = socket_client.send_and_receive ((unsigned char*)evt_message.c_str (), evt_message.size () + 1,
                    &p_answer, &answer_size);
        }

        sleep (2);
    }*/

    if (is_going_on)
        if (raspberry_board_setup ())
            is_going_on = 0;

    //sensor_dht11_setup ();
    sensor_relay_water_pump_setup ();
    //liquid_level_setup ();

    int count = 0;
    while (is_going_on)
    {
        if (count == 0)
        {
            sensor_relay_water_pump_start ();

            unsigned int hum; int temp;
            //sensor_dht11_get_data (&temp, &hum);
            dht22_get_data (4, &hum, &temp);
            DEBUG_LOG_INFO ("sensor_dht11_get_data %2.1f %%   %2.1f C", (float)hum / 10, (float)temp / 10);

            sleep (1);
            sensor_relay_water_pump_stop ();
        }

        //get_liquid_level ();
        sleep (1);

        count ++;
        if (count == 5)
            count = 0;
    }

    DEBUG_LOG_INFO ("app exit");
    return 0;
}
