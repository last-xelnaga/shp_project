
#ifndef SENSOR_EVENT_CLASS_HPP
#define SENSOR_EVENT_CLASS_HPP

#include <thread>


class sensor_event_class
{
public:
    typedef void (*f_event_callback)(void* p_user_data);

private:
    // this is our thread identifier
    std::thread m_worker_thread;
    volatile int is_time_to_close;

    unsigned int mi_gpio_num;

    f_event_callback p_event_on_callback;
    f_event_callback p_event_off_callback;

    void* mp_user_data;

private:
    void working_thread (
            void);

    void event_on (
            void);

    void event_off (
            void);

public:
    sensor_event_class (
            const unsigned int gpio_num);

    void set_event_callback (
            f_event_callback p_event_on_callback_,
            f_event_callback p_event_off_callback_,
            const void* p_user_data);

    ~sensor_event_class (
            void);
};

#endif // ifndef SENSOR_EVENT_CLASS_HPP
