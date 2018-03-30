
#include "client_socket_class.hpp"
#include "socket_common.h"
//#include "message.hpp"
//#include "debug.hpp"

#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>


static int connect_to_server (
        const int fd,
        const char* p_server_name,
        const unsigned int port_number)
{
    int result = 0;
    struct hostent* p_host = NULL;
    struct sockaddr_in socket_address;
    //DEBUG_LOG_TRACE_BEGIN

    if (fd < 0)
    {
        //DEBUG_LOG_MESSAGE ("invalid socket param");
        result = -1;
    }

    if (result == 0)
    {
        p_host = gethostbyname (p_server_name);
        if (p_host == NULL)
        {
            //DEBUG_LOG_MESSAGE ("gethostbyname call failed: %s", strerror (errno));
            result = -1; //RESULT_SOCKET_ERROR;
        }
    }

    if (result == 0)
    {
        memcpy ((char*)&socket_address.sin_addr, p_host->h_addr, p_host->h_length);
        socket_address.sin_family = AF_INET;
        socket_address.sin_port = htons (port_number);
    }

    if (result == 0)
    {
        int res = connect (fd, (struct sockaddr*)&socket_address, sizeof (sockaddr_in));
        if (res < 0)
        {
            //DEBUG_LOG_MESSAGE ("connect call failed: %s", strerror (errno));
            result = -1;
        }
    }

    //DEBUG_LOG_TRACE_END (result)
    return result;
}

client_socket_class::client_socket_class (
        void)
{
    connect_retry_sleep = 2;
    try_count_max = 3;

    write_timeout = 10;
    read_timeout = 60;
}

int client_socket_class::connect (
        const char* p_server_name,
        const unsigned int port_number)
{
    int result = 0;
    //DEBUG_LOG_TRACE_BEGIN

    if (result == 0)
    {
        //result = create_socket (&socket_fd);
        socket_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (socket_fd < 0)
        {
            //DEBUG_LOG_MESSAGE ("socket call failed: %s", strerror (errno));
            result = -1;
        }
    }

    // connect
    if (result == 0)
    {
        result = -1; //RESULT_SOCKET_ERROR;
        unsigned int try_count = try_count_max;

        while (result != 0 && try_count)
        {
            result = connect_to_server (socket_fd, p_server_name, port_number);
            try_count --;

            if (result != 0)
            {
                if (try_count == 0)
                {
                    // no attempts left
                    //DEBUG_LOG_MESSAGE ("connect failed. no attempts left");
                    result = -1; //RESULT_SOCKET_ERROR;
                }
                else
                {
                    // give it an another chance
                    sleep (connect_retry_sleep);
                }
            }
        }
    }

    // set non-blocking mode
    if (result == 0)
    {
        result = set_blocking (socket_fd, true);
    }

    // make a clean up if needed
    if (result != 0)
    {
        if (socket_fd >= 0)
        {
            ::close (socket_fd);
        }
        socket_fd = -1;
    }

    //DEBUG_LOG_TRACE_END (result)
    return result;
}

int client_socket_class::send_and_receive (
        const unsigned char* p_buffer,
        const unsigned int size,
        unsigned char** p_answer,
        unsigned int* answer_size)
{
    int result = 0;
    //message_class::message_header_t header;
    //DEBUG_LOG_TRACE_BEGIN

    if (result == 0)
    {
        //unsigned char* p_data = NULL;
        //unsigned int data_length = 0;
        //p_message->get_header (&p_data, &data_length);
        result = send_data (socket_fd, (unsigned char*)&size, sizeof (int), write_timeout);
    }

    if (result == 0)
    {
        //unsigned char* p_data = NULL;
        //unsigned int data_length = 0;
        //p_message->get_payload (&p_data, &data_length);
        //if (data_length)
        {
            result = send_data (socket_fd, p_buffer, size, write_timeout);
        }
    }

    if (result == 0)
    {
        result = recv_data (socket_fd, (unsigned char*) answer_size, sizeof (int), read_timeout);
    }

    if (result == 0)
    {
        *p_answer /*[]*/ = new unsigned char (*answer_size);
    }

    if (result == 0)
    {
        //unsigned char* p_data = NULL;
        //unsigned int data_length = 0;
        //(*p_answer)->get_payload (&p_data, &data_length);
        //if (data_length)
        //{
            result = recv_data (socket_fd, *p_answer, *answer_size, read_timeout);
        //}
    }

    //DEBUG_LOG_TRACE_END (result);
    return result;
}

void client_socket_class::close (
        void)
{
    //DEBUG_LOG_TRACE_BEGIN

    if (socket_fd >= 0)
    {
        ::close (socket_fd);
    }
    socket_fd = -1;

    //DEBUG_LOG_TRACE_END (0)
}

client_socket_class::~client_socket_class (
        void)
{
    //DEBUG_LOG_TRACE_BEGIN

    client_socket_class::close ();

    //DEBUG_LOG_TRACE_END (0)
}
