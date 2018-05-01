
#include "client_socket_class.hpp"
#include "log.h"
#include "network_manager_class.hpp"

#define SERVER_IP       "127.0.0.1"
#define SERVER_PORT     5000


struct pws_message
{
    unsigned char level;  // [0..100]
    //time_t
} ;


network_manager_class::network_manager_class (
        void)
{

}

void network_manager_class::enqueue_message (
        void)
{
    client_socket_class socket_client;
    socket_client.connect (SERVER_IP, SERVER_PORT);

    char buffer [] = "notification";
    unsigned char* p_answer = NULL;
    unsigned int answer_size = 0;

    socket_client.send_and_receive ((unsigned char*)buffer, strlen (buffer) + 1,
            &p_answer, &answer_size);
    socket_client.close ();

    if (answer_size)
        DEBUG_LOG_INFO ("%s", p_answer);

    if (p_answer)
        delete p_answer;

    //return 0;
}
