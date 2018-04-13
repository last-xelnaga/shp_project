
#include "log.h"
#include "client_socket_class.hpp"


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
        DEBUG_LOG_INFO ("%s", p_answer);

    if (p_answer)
        delete p_answer;

    return 0;
}
