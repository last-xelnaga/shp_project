
#include "log.h"
#include "fcm_messaging_class.hpp"
#include "server_socket_class.hpp"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>


volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
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
            printf ("%s\n", buffer);
            
            //fcm_messaging_class::get_instance ().register_message ("test_title", "test_body");

            char answer [] = "OK";
            p_client->send_message ((unsigned char*)answer, 3);
        }

        if (buffer)
            delete [] buffer;
        buffer = NULL;
        size = 0;
    }
}

int main (
        void)
{
    signal (SIGINT, exit_function);

    server_socket_class server_socket;
    server_socket.start (5000, &server_worker);

    while (is_going_on)
    {
        sleep (1);
    }

    DEBUG_LOG_INFO ("quiting...");
    server_socket.terminate ();

    return 0;
}
