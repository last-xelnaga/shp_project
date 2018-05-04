
#include "client_socket_class.hpp"
#include "log.h"
#include "network_manager_class.hpp"
#include "settings_class.hpp"


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
        std::string message)
{
    printf ("%s\n", message.c_str ());
    
    return;
    
    client_socket_class socket_client;
    socket_client.connect (settings_class::get_instance ().get_server_name ().c_str (),
            settings_class::get_instance ().get_server_port ());

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
