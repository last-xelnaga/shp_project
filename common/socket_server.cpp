
#include "socket_server.hpp"
#include "socket.hpp"
//#include "message.hpp"
//#include "debug.hpp"

#include <unistd.h>
//#include <netdb.h>
//#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
//#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>


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
    //message_class::message_header_t header;
    //DEBUG_LOG_TRACE_BEGIN

    if (result == 0)
    {
        result = recv_data (accept_fd, (unsigned char*) size, sizeof (int), read_timeout);
    }

    if (result == 0)
    {
        *buffer /*[]*/ = new unsigned char (*size);
    }

    if (result == 0)
    {
        //unsigned char* p_data = NULL;
        //unsigned int data_length = 0;
        //(*p_message)->get_payload (&p_data, &data_length);
        if (*size)
        {
            result = recv_data (accept_fd, *buffer, *size, read_timeout);
        }
    }

    //DEBUG_LOG_TRACE_END (result)
    return result;
}

int server_socket_class::server_client_class::send_message (
        const unsigned char* p_buffer,
        const unsigned int size)
{
    int result = 0;
    //DEBUG_LOG_TRACE_BEGIN

    if (result == 0)
    {
        unsigned char* p_data = (unsigned char*)&size;
        unsigned int data_length = sizeof (int);
        //p_message->get_header (&p_data, &data_length);
        result = send_data (accept_fd, p_data, data_length, write_timeout);
    }

    if (result == 0)
    {
        //unsigned char* p_data = NULL;
        //unsigned int data_length = 0;
        //p_message->get_payload (&p_data, &data_length);
        //if (data_length)
        //{
            result = send_data (accept_fd, p_buffer, size, write_timeout);
        //}
    }

    //DEBUG_LOG_TRACE_END (result)
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






/*struct __attribute__((__packed__)) huj_t
{
    server_socket_class* p_server;
    server_socket_class::server_routine p_func;
    //unsigned int port;
};*/


void* server_socket_class::task1 (
    void* p_huj)
{
    //struct huj_t* p_real_huj = (struct huj_t*)p_huj;
    server_socket_class* p_server = (server_socket_class*)p_huj;//p_real_huj->p_server;

    char is_time_close = 0;

    do {
        switch (p_server->state)
        {
            //case ready_to_create:
            //    break;
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


    /*do
    {
        printf ("start the accept loop\n");

        int accept_fd = -1;
        p_server->accept_client (&accept_fd);

        if (accept_fd < 0)
            break;

        printf ("register new client\n");
        server_client_class* p_client = new server_client_class (p_real_huj->p_func, accept_fd);
        p_client->run ();

    } while (1);*/

    printf ("end the accept loop\n");
    return NULL;
}

int server_socket_class::bind_on_server (
        void)
{
    int result = 0;
    struct sockaddr_in socket_address;
    //DEBUG_LOG_TRACE_BEGIN

    if (socket_fd < 0)
    {
        //DEBUG_LOG_MESSAGE ("invalid socket param");
        printf ("invalid socket param\n");
        result = -1;
    }

    if (result == 0)
    {
        int flag = 1;
        int res = setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag));
        if (res < 0)
        {
            //DEBUG_LOG_MESSAGE ("setsockopt call failed: %s", strerror (errno));
            printf ("setsockopt call failed: %s\n", strerror (errno));
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
            //DEBUG_LOG_MESSAGE ("bind call failed: %s", strerror (errno));
            printf ("bind call failed: %s\n", strerror (errno));
            result = -1;
        }
    }

    if (result == 0)
    {
        int res = listen (socket_fd, listen_queue);
        if (res < 0)
        {
            //DEBUG_LOG_MESSAGE ("listen call failed: %s", strerror (errno));
            printf ("listen call failed: %s\n", strerror (errno));
            result = -1;
        }
    }

    //DEBUG_LOG_TRACE_END (result)
    return result;
}

server_socket_class::server_socket_class (
        void)
{
    socket_fd = -1;
    listen_queue = 5;
    //terminated = false;

    state = ready_to_bind;
    mi_port_number = 0;
}

//static struct huj_t huj;

int server_socket_class::start (
        unsigned int port_number,
        server_routine p_func)
{
    int result = 0;

    if (state != ready_to_bind)
    {
        printf ("invalid socket state\n");
        result = -1;
    }

mi_port_number = port_number;
mp_func = p_func;

    // make a clean up if needed
    if (result == 0)
    {
        //huj.p_server = this;
        //huj.p_func = p_func;
        //huj.port = port_number;
        pthread_create ((pthread_t*)&listener, NULL, task1, (void*)this/*&huj*/);
    }

    return result;
}

int server_socket_class::bind_and_listen (
        /*unsigned int port_number,
        server_routine p_func*/void)
{
    int result = 0;
    //DEBUG_LOG_TRACE_BEGIN

    if (result == 0)
    {
        socket_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (socket_fd < 0)
        {
            //DEBUG_LOG_MESSAGE ("socket call failed: %s", strerror (errno));
            printf ("socket call failed: %s\n", strerror (errno));
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



    //DEBUG_LOG_TRACE_END (result)
    return result;
}

int server_socket_class::accept_client (
/*int* accept_fd*/void)
{
    int result = 0;
        int accept_fd = -1;
    //DEBUG_LOG_TRACE_BEGIN

    //if (socket_fd < 0)
    //{
        //DEBUG_LOG_MESSAGE ("socket is not ready");
    //    printf ("socket is not ready\n");
    //    result = -1;//RESULT_INVALID_STATE;
    //}

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
            //DEBUG_LOG_MESSAGE ("accept call failed: %s", strerror (errno));
            printf ("accept call failed: %s\n", strerror (errno));
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

    //DEBUG_LOG_TRACE_END (result)
    return result;
}

int server_socket_class::add_client_to_list (
        int accept_fd)
{
    printf ("register new client\n");

    server_client_class* p_client = new server_client_class (mp_func, accept_fd);
    p_client->run ();

    return 0;
}

int server_socket_class::remove_finished (
        void)
{
return 0;
}

void server_socket_class::terminate (
        void)
{
    //DEBUG_LOG_TRACE_BEGIN

    state = ready_to_close;

    printf ("waiting for join\n");
    shutdown (socket_fd, 0);
    pthread_join(listener, NULL);
    printf ("joined\n");
    listener = -1;

    if (socket_fd >= 0)
    {
        close (socket_fd);
    }
    socket_fd = -1;

    //DEBUG_LOG_TRACE_END (0)
}

server_socket_class::~server_socket_class (
        void)
{
    //DEBUG_LOG_TRACE_BEGIN

    if (socket_fd >= 0)
        server_socket_class::terminate ();

    //DEBUG_LOG_TRACE_END (0)
}
