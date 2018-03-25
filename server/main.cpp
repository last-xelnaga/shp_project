
#include "socket_server.hpp"
#include <stdio.h>
#include <unistd.h>

int send_notification (
    void);

void server_worker (
    server_socket_class::server_client_class* p_client)
{
    unsigned char* buffer = NULL;
    unsigned int size = 0;

    p_client->recv_message (&buffer, &size);

    if (size)
    {
        printf ("%s\n", buffer);

        send_notification ();

        char answer [] = "huj";
        p_client->send_message ((unsigned char*)answer, 4);
    }
}

int main (
    void)
{
    server_socket_class server_socket;
    server_socket_class::server_routine p_func = &server_worker;
    server_socket.start (5000, p_func);

    sleep (3);
    printf ("press q to exit\n");
    do
    {
        usleep (1000);
    }
    while (getchar() != 'q');

    printf ("after loop\n");
    server_socket.terminate ();

    return 0;
}
