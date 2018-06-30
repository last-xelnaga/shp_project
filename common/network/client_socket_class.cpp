
#include "client_socket_class.hpp"
#include "log.h"
#include "socket_common.h"

#include <errno.h>
#include <linux/tcp.h>
#include <netdb.h>
#include <unistd.h>


#define SOCKET_WRITE_TIMEOUT        10
#define SOCKET_READ_TIMEOUT         60
#define SOCKET_ADD_SYN_COUNT        1

void parse_url (
        const std::string proxy,
        std::string& host_name,
        unsigned int& host_port)
{
    int offset = 0;

    // check the protocol
    offset = offset == 0 && proxy.compare (0, 8, "https://") == 0 ? 8 : offset;
    offset = offset == 0 && proxy.compare (0, 7, "http://" ) == 0 ? 7 : offset;

    // look for port delimeter
    size_t port_pos = proxy.find_first_of (':', offset + 1 );
    host_name = port_pos == std::string::npos ? proxy.substr (offset) : proxy.substr (offset, port_pos - offset);
    host_port = std::stoi (port_pos == std::string::npos ? offset == 7 ? "80" : "443" : proxy.substr (port_pos + 1));

    LOG_INFO ("host %s:%d", host_name.c_str (), host_port);
}

int client_socket_class::connect_to_server (
        void)
{
    int result = 0;
    struct sockaddr_in socket_address;

    // prepare the proxy
    proxy_payload = "CONNECT " + server_name + ":" + std::to_string (mi_port_number) + " HTTP/1.0\n\n";

    if (proxy_name.size ())
    {
        parse_url (proxy_name, server_name, mi_port_number);
    }

    LOG_INFO ("server_name %s", server_name.c_str ());

    if (result == 0)
    {
        socket_address.sin_family = AF_INET;
        socket_address.sin_port = htons (mi_port_number);

        struct hostent* p_host = gethostbyname (server_name.c_str ());
        if (p_host == NULL)
        {
            LOG_ERROR ("gethostbyname call failed: %s", strerror (errno));
            result = -1;
        }
        else
        {
            memcpy ((char*)&socket_address.sin_addr, p_host->h_addr, p_host->h_length);
        }
    }

    if (result == 0)
    {
        int res = connect (socket_fd, (struct sockaddr*)&socket_address, sizeof (sockaddr_in));
        if (res < 0)
        {
            LOG_ERROR ("connect call failed: %s", strerror (errno));
            result = -1;
        }
    }

    if (result == 0 && proxy_name.size ())
    {
        char p_answer [1024] = { 0 };
        unsigned int answer_size = sizeof (p_answer);

        send (socket_fd, proxy_payload.c_str (), proxy_payload.size (), 0);
        int received = recv (socket_fd, p_answer, answer_size, 0);
        if (received > 12)
        {
            p_answer [12] = 0;
            if (strcmp (p_answer, "HTTP/1.1 200") != 0)
            {
                LOG_ERROR ("proxy said %s", p_answer);
                result = -1;
            }
        }
        else
        {
            LOG_ERROR ("connect to proxy failed");
            result = -1;
        }
    }

    return result;
}

client_socket_class::client_socket_class (
        void)
{
    socket_fd = -1;
}

int client_socket_class::open_connection (
        const char* p_server_name,
        const unsigned int port_number)
{
    int result = 0;

    if (socket_fd > 0)
    {
        LOG_ERROR ("wrong socket state");
        result = -1;
    }

    if (result == 0)
    {
        server_name = std::string (p_server_name);
        mi_port_number = port_number;
    }

    // create socket
    if (result == 0)
    {
        socket_fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (socket_fd < 0)
        {
            LOG_ERROR ("socket call failed: %s", strerror (errno));
            result = -1;
        }
    }

    // set SYN packets count for the connect retries
    if (result == 0)
    {
        int syn_packets = SOCKET_ADD_SYN_COUNT;
        int res = setsockopt (socket_fd, IPPROTO_TCP, TCP_SYNCNT, &syn_packets, sizeof (syn_packets));
        if (res < 0)
        {
            LOG_ERROR ("setsockopt call failed: %s", strerror (errno));
            result = -1;
        }
    }

    // check for proxy
    if (result == 0)
    {
        char* p_http_proxy = getenv ("http_proxy");
        if (p_http_proxy != NULL)
        {
            LOG_INFO ("proxy: \"%s\"", p_http_proxy);
            proxy_name = std::string (p_http_proxy);
        }
    }

    if (result == 0)
    {
        if (server_name == "localhost" && proxy_name.size ())
        {
            LOG_INFO ("skip proxy for localhost");
            proxy_name = "";
        }
    }

    // connect
    if (result == 0)
    {
        result = connect_to_server ();
    }

    // set non-blocking mode
    if (result == 0)
    {
        result = set_blocking (socket_fd, true);
    }

    // make a clean up if needed
    if (result < 0)
    {
        if (socket_fd >= 0)
            close (socket_fd);
        socket_fd = -1;
    }

    return result;
}

int client_socket_class::send_and_receive (
        const unsigned char* p_buffer,
        const unsigned int size,
        unsigned char** p_answer,
        unsigned int* answer_size)
{
    int result = 0;

    if (result == 0)
    {
        result = send_data (socket_fd, (unsigned char*)&size, sizeof (int), SOCKET_WRITE_TIMEOUT);
    }

    if (result == 0)
    {
        result = send_data (socket_fd, p_buffer, size, SOCKET_WRITE_TIMEOUT);
    }

    if (result == 0)
    {
        result = recv_data (socket_fd, (unsigned char*) answer_size, sizeof (int), SOCKET_READ_TIMEOUT);
    }

    if (result == 0)
    {
        *p_answer = new unsigned char [*answer_size];
    }

    if (result == 0)
    {
        result = recv_data (socket_fd, *p_answer, *answer_size, SOCKET_READ_TIMEOUT);
    }

    return result;
}

void client_socket_class::close_connection (
        void)
{
    if (socket_fd != -1)
    {
        close (socket_fd);
    }
    socket_fd = -1;
}

client_socket_class::~client_socket_class (
        void)
{
    close_connection ();
}
