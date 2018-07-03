
#include "log.h"
#include "rpi_gpio.h"
#include "sensor_event_class.hpp"
#include "time_utils.h"


void sensor_event_class::working_thread (
        void)
{
    if (!is_time_to_close)
    {
        //printf ("waiting for event ...\n");
        while (get_bus_state (mi_gpio_num) == HIGH)
            sleep_milliseconds (200);
            //printf ("button pressed\n") ;
        event_on ();

        while (get_bus_state (mi_gpio_num) == LOW)
            sleep_milliseconds (200);
        //printf ("button released\n") ;
        event_off ();
    }
}

void sensor_event_class::event_on (
        void)
{
    if (p_event_on_callback)
        p_event_on_callback (mp_user_data);
}

void sensor_event_class::event_off (
        void)
{
    if (p_event_off_callback)
        p_event_off_callback (mp_user_data);
}


sensor_event_class::sensor_event_class (
        const unsigned int gpio_num)
{
    mi_gpio_num = gpio_num;

    p_event_on_callback = NULL;
    p_event_off_callback = NULL;

    mp_user_data = NULL;

    is_time_to_close = 1;
}

void sensor_event_class::set_event_callback (
        f_event_callback p_event_on_callback_,
        f_event_callback p_event_off_callback_,
        const void* p_user_data)
{
    if (is_time_to_close)
        return;

    p_event_on_callback = p_event_on_callback_;
    p_event_off_callback = p_event_off_callback_;

    mp_user_data = (void*)p_user_data;

    set_pin_direction (mi_gpio_num, INPUT);
    set_pin_voltage (mi_gpio_num, LOW);

    is_time_to_close = 0;
    m_worker_thread = std::thread (&sensor_event_class::working_thread, this);
}

sensor_event_class::~sensor_event_class (
        void)
{
    is_time_to_close = 1;
    m_worker_thread.join ();
}
