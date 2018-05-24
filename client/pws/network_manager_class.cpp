
#include "client_socket_class.hpp"
#include "log.h"
#include "network_manager_class.hpp"
#include "settings_class.hpp"

#include <pthread.h>
#include <unistd.h>


pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// private methods
network_manager_class::network_manager_class (
        void)
{
    server_last_connection_time = time (NULL);

    is_worker_active = 1;
    pthread_create ((pthread_t*)&listener, NULL, network_manager_worker, (void*)this);
}

void* network_manager_class::network_manager_worker (
        void* p_class)
{
    network_manager_class* p_manager = (network_manager_class*)p_class;

    while (p_manager->is_worker_active)
    {
        p_manager->process_batch ();
        sleep (1);
    }

    DEBUG_LOG_INFO ("end of the loop");
    return NULL;
}

void network_manager_class::process_batch (
        void)
{
    if (server_last_connection_time + settings_class::get_instance ().get_server_batch_cycle () > (unsigned long) time (NULL))
    {
        //DEBUG_LOG_INFO ("process_batch early");
        return;
    }

    if (m_queue.size () == 0)
    {
        server_last_connection_time = time (NULL);
        DEBUG_LOG_INFO ("no messages, new connection_time is %ld", server_last_connection_time);
        return;
    }

    client_socket_class socket_client;
    int status = socket_client.connect (settings_class::get_instance ().get_server_name ().c_str (),
            settings_class::get_instance ().get_server_port ());

    if (status == 0)
    {
        // start work with queue, lock the mutex
        pthread_mutex_lock (&queue_mutex);

        DEBUG_LOG_INFO ("process_batch %d messages", m_queue.size ());

        auto it = m_queue.begin ();
        while (it != m_queue.end ())
        {
            //DEBUG_LOG_INFO ("process message\n%s", message);

            // try to send
            unsigned char* p_answer = NULL;
            unsigned int answer_size = 0;
            status = socket_client.send_and_receive ((unsigned char*)it->c_str (), it->size () + 1,
                    &p_answer, &answer_size);
            if (status == 0)
            {
                //if (answer_size)
                //    DEBUG_LOG_INFO ("%s", p_answer);

                if (p_answer)
                    delete [] p_answer;

                m_queue.erase (it ++);
            }
            else
                ++ it;
        }

        // we have finished our work with queue
        pthread_mutex_unlock (&queue_mutex);

        socket_client.close ();
    }

    server_last_connection_time = time (NULL);
    DEBUG_LOG_INFO ("new server_last_connection_time %d", server_last_connection_time);
}

static char* get_time_str (
        time_t now)
{
    struct tm* tm = localtime (&now);

    static char mon_name [12][4] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    static char result [21];

    sprintf (result, "%d %s %.2d %.2d:%.2d:%.2d",
        1900 + tm->tm_year,
        mon_name [tm->tm_mon],
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec);
    return result;
}

// public methods
void network_manager_class::enqueue_message (
        std::string message)
{
    DEBUG_LOG_INFO ("add new message to the queue");

    time_t now = time (NULL);


    std::string evt_message = "{\n";
    evt_message += "  \"client\" : \"pws\",\n";
    evt_message += "  \"evt_time\" : \"" + std::string (get_time_str (now)) + "\",\n";
    evt_message += "  \"evt_time_unix\" : " + std::to_string (now) + ",\n";
    evt_message += message;
    evt_message += "}\n";

    // start work with queue, lock the mutex
    pthread_mutex_lock (&queue_mutex);

    m_queue.push_back (evt_message);

    // we have finished our work with queue
    pthread_mutex_unlock (&queue_mutex);
}

network_manager_class::~network_manager_class (
        void)
{
    is_worker_active = 0;
    pthread_join (listener, NULL);
    DEBUG_LOG_INFO ("worker thread joined");
}
