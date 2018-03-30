
#include "client_socket_class.hpp"
#include <stdio.h>
#include <string.h>

int main (
    void)
{
    client_socket_class socket_client;
    socket_client.connect ("127.0.0.1", 5000);

    char buffer [] = "notification";
    unsigned char* p_answer = NULL;
    unsigned int answer_size = 0;

    socket_client.send_and_receive ((unsigned char*)buffer, strlen (buffer) + 1,
            &p_answer, &answer_size);
    socket_client.close ();

    if (answer_size)
        printf ("%s\n", p_answer);

    if (p_answer)
        delete p_answer;

    return 0;
}
