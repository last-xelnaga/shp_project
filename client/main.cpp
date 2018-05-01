
#include "network_manager_class.hpp"
#include "log.h"
#include "sensors.h"
#include <string>
#include <time.h>
#include <unistd.h>


bool is_time_for_watering (
        void)
{
    return false;
}

void do_watering (
        void)
{
    /*{
        "test3" : {
            "x" : 123.456
        },
        "test4" : [
            1,
            2,
            3,
            {
                "z" : 12345
            }
        ],
        "test1" : "hello world",
        "test2" : "BLAH\uD840\uDC8ABLAH"
    }*/

    std::string message = "{\n";
    message += "  time : " + std::to_string (time (NULL)) + ",\n";

    message += "  \"watering\" : {\n";

    // get water level before watering
    int limit = get_liquid_level ();
    message += "    \"limit\" : " + std::to_string (limit) + ",\n";

    if (limit > 1)
    {
        // start the pump
        if (water_pump_start () == 0)
        {
            // wait enough for the 100 ml
            //sleep (90);

            // stop the pump
            water_pump_stop ();

            message += "    \"status\" : \"OK\"\n";
        }
        else
        {
            message += "    \"status\" : \"failed to start pump\"\n";
        }
    }
    else
    {
        message += "    \"status\" : \"water level too low\"\n";
    }
    message += "  }\n}\n";

    printf ("%s\n", message.c_str ());

    // send the message
    //network_manager_class::get_instance().enqueue_message ();
}

bool is_time_for_temperature (
        void)
{
    return false;
}

void do_temperature_check (
        void)
{
    // get the current temperature
    get_temperature ();

    // and humidity
    get_humidity ();

    // send the message
    network_manager_class::get_instance().enqueue_message ();
}

int main (
        void)
{
do_watering ();
return 0;

    while (1)
    {
        if (is_time_for_watering ())
        {
            do_watering ();
        }

        if (is_time_for_temperature ())
        {
            do_temperature_check ();
        }

        sleep (1);
    }

    return 0;
}
