
#include "log.h"
#include "server_socket_class.hpp"
#include "server_worker.hpp"

#include <signal.h>
#include <unistd.h>

#define SERVER_PORT         5000

volatile sig_atomic_t is_going_on = 1;

void exit_function (
        int sig)
{
    // happy compiler
    if (sig) {}

    is_going_on = 0;
}

int main (
        void)
{
    signal (SIGINT, exit_function);

    server_socket_class server_socket;
    server_socket.start (SERVER_PORT, &server_worker);

    while (is_going_on)
    {
        show_table ();
        sleep (1);
    }

    DEBUG_LOG_INFO ("quiting...");
    server_socket.terminate ();

    return 0;
}
