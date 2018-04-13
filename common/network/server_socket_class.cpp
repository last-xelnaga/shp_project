
#include "log.h"
#include "server_socket_class.hpp"
#include "socket_common.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>


server_socket_class::server_client_class::server_client_class (
        server_routine p_func,
        int fd)
{
    accept_fd = fd;
    p_routine = p_func;

    write_timeout = 10;
    read_timeout = 60;
}

void server_socket_class::server_client_class::run (
        void)
{
    p_routine (this);
}

int server_socket_class::server_client_class::recv_message (
        unsigned char** buffer,
        unsigned int* size)
{
    int result = 0;

    if (result == 0)
    {
        result = recv_data (accept_fd, (unsigned char*) size, sizeof (int), read_timeout);
    }

    if (result == 0)
    {
        *buffer = new unsigned char (*size);
    }

    if (result == 0)
    {
        if (*size)
        {
            result = recv_data (accept_fd, *buffer, *size, read_timeout);
        }
    }

    return result;
}

int server_socket_class::server_client_class::send_message (
        const unsigned char* p_buffer,
        const unsigned int size)
{
    int result = 0;

    if (result == 0)
    {
        unsigned char* p_data = (unsigned char*)&size;
        unsigned int data_length = sizeof (int);
        result = send_data (accept_fd, p_data, data_length, write_timeout);
    }

    if (result == 0)
    {
        result = send_data (accept_fd, p_buffer, size, write_timeout);
    }

    return result;
}

server_socket_class::server_client_class::~server_client_class (
        void)
{
    if (accept_fd >= 0)
    {
        close (accept_fd);
    }
    accept_fd = -1;
}

void* server_socket_class::task1 (
        void* p_huj)
{
    server_socket_class* p_server = (server_socket_class*)p_huj;

    char is_time_close = 0;

    do {
        switch (p_server->state)
        {
            case ready_to_bind:
                p_server->bind_and_listen ();
                break;
            case ready_to_accept:
                p_server->accept_client ();
                break;
            case ready_to_close:
                is_time_close = 1;
                break;
        }
    } while (!is_time_close);

    DEBUG_LOG_INFO ("end the accept loop");
    return NULL;
}

int server_socket_class::bind_on_server (
        void)
{
    int result = 0;
    struct sockaddr_in socket_address;

    if (socket_fd < 0)
    {
        DEBUG_LOG_ERROR ("invalid socket param");
        result = -1;
    }

    if (result == 0)
    {
        int flag = 1;
        int res = setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag));
        if (res < 0)
        {
            DEBUG_LOG_ERROR ("setsockopt call failed: %s", strerror (errno));
            result = -1;
        }
    }

    if (result == 0)
    {
        socket_address.sin_family = AF_INET;
        socket_address.sin_addr.s_addr = htonl (INADDR_ANY);
        socket_address.sin_port = htons (mi_port_number);
    }

    if (result == 0)
    {
        int res = bind (socket_fd, (struct sockaddr *) &socket_address, sizeof (struct sockaddr_in));
        if (res < 0)
        {
            DEBUG_LOG_ERROR ("bind call failed: %s", strerror (errno));
            result = -1;
        }
    }

    if (result == 0)
    {
        int res = listen (socket_fd, listen_queue);
        if (res < 0)
        {
            DEBUG_LOG_ERROR ("listen call failed: %s", strerror (errno));
            result = -1;
        }
    }

    return result;
}

server_socket_class::server_socket_class (
        void)
{
    socket_fd = -1;
    listen_queue = 5;
    state = ready_to_bind;
    mi_port_number = 0;

    client_class = nullptr;
}

int server_socket_class::start (
        unsigned int port_number,
        server_routine p_func)
{
    int result = 0;

    if (state != ready_to_bind)
    {
        DEBUG_LOG_ERROR ("invalid socket state");
        result = -1;
    }

mi_port_number = port_number;
mp_func = p_func;

    // make a clean up if needed
    if (result == 0)
    {
        pthread_create ((pthread_t*)&listener, NULL, task1, (void*)this);
    }

    return result;
}

int server_socket_class::bind_and_listen (
        void)
{
    int result = 0;

    if (result == 0)
    {
        socket_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (socket_fd < 0)
        {
            DEBUG_LOG_ERROR ("socket call failed: %s", strerror (errno));
            result = -1;
        }
    }

    if (result == 0)
    {
        result = bind_on_server ();
        if (result == 0)
        {
            state = ready_to_accept;
        }
        else
        {
            // make a clean up if needed
            close (socket_fd);
            socket_fd = -1;
        }
    }

    return result;
}

int server_socket_class::accept_client (
        void)
{
    int result = 0;
    int accept_fd = -1;

    // set blocking mode
    if (result == 0)
    {
        result = set_blocking (socket_fd, false);
    }

    if (result == 0)
    {
        accept_fd = accept (socket_fd, NULL, NULL);
        if (accept_fd < 0)
        {
            DEBUG_LOG_ERROR ("accept call failed: %s", strerror (errno));
            result = -1;
        }
    }

    // set non-blocking mode
    if (result == 0)
    {
        result = set_blocking (socket_fd, true);
    }

    if (result == 0)
    {
        add_client_to_list (accept_fd);
    }

    return result;
}

int server_socket_class::add_client_to_list (
        int accept_fd)
{
    DEBUG_LOG_INFO ("register new client");

    remove_finished ();

    client_class = new server_client_class (mp_func, accept_fd);
    client_class->run ();

    return 0;
}

int server_socket_class::remove_finished (
        void)
{
    if (client_class != nullptr)
        delete client_class;

    return 0;
}

void server_socket_class::terminate (
        void)
{
    state = ready_to_close;

    DEBUG_LOG_INFO ("waiting for join");
    shutdown (socket_fd, 0);
    pthread_join(listener, NULL);
    DEBUG_LOG_INFO ("joined");
    listener = -1;

    if (socket_fd >= 0)
    {
        close (socket_fd);
    }
    socket_fd = -1;

    remove_finished ();
}

server_socket_class::~server_socket_class (
        void)
{
    if (socket_fd >= 0)
        server_socket_class::terminate ();
}
