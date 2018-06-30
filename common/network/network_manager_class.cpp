
#include "client_socket_class.hpp"
#include "log.h"
#include "network_manager_class.hpp"
//#include "sensor_buzzer.h"
#include "settings_class.hpp"
#include "time_utils.h"

#include <pthread.h>
#include <unistd.h>

// check queue for new messages every minute
#define QUEUE_PROCESS_CYCLE         60

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
        if (p_manager->server_last_connection_time + QUEUE_PROCESS_CYCLE < (unsigned long) time (NULL))
            p_manager->flush ();

        sleep (1);
    }

    LOG_INFO ("end of the loop");
    return NULL;
}

void network_manager_class::enqueue_message (
        std::string client,
        std::string type,
        const int status,
        std::vector <std::string> data)
{
    LOG_INFO ("add new message to the queue");

    std::string evt_message = "{\n";
    evt_message += "  \"client\" : \"" + client + "\",\n";
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

    // add message to the queue
    pthread_mutex_lock (&queue_mutex);
    m_queue.push_back (evt_message);
    pthread_mutex_unlock (&queue_mutex);
}

void network_manager_class::flush (
        void)
{
    // start work with queue, lock the mutex
    pthread_mutex_lock (&queue_mutex);

    do
    {
        if (m_queue.size () == 0)
        {
            server_last_connection_time = time (NULL);
            LOG_INFO ("no messages, new connection_time is %ld", server_last_connection_time);
            break;
        }

        client_socket_class socket_client;
        int status = socket_client.open_connection (settings_class::get_value_for ("server_name").c_str (),
                std::stoi (settings_class::get_value_for ("server_port")));
        if (status != 0)
        {
            LOG_INFO ("failed to open connection");
            break;
        }

        LOG_INFO ("process_batch %d messages", m_queue.size ());

        auto it = m_queue.begin ();
        while (it != m_queue.end ())
        {
            //LOG_INFO ("process message\n%s", message);

            // try to send
            unsigned char* p_answer = NULL;
            unsigned int answer_size = 0;
            status = socket_client.send_and_receive ((unsigned char*)it->c_str (), it->size () + 1,
                    &p_answer, &answer_size);
            if (status == 0)
            {
                //if (answer_size)
                //    LOG_INFO ("%s", p_answer);

                if (p_answer)
                    delete [] p_answer;

                m_queue.erase (it ++);
            }
            else
                ++ it;
        }

        socket_client.close_connection ();

        server_last_connection_time = time (NULL);
        LOG_INFO ("new server_last_connection_time %d", server_last_connection_time);
    }
    while (0);

    // we have finished our work with queue
    pthread_mutex_unlock (&queue_mutex);
}

network_manager_class::~network_manager_class (
        void)
{
    is_worker_active = 0;
    pthread_join (listener, NULL);
    LOG_INFO ("worker thread joined");
}
