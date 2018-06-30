
#ifndef SOCKET_CLIENT_CLASS_HPP
#define SOCKET_CLIENT_CLASS_HPP

#include <string>


class client_socket_class
{
private:
    int socket_fd;

    std::string server_name;
    unsigned int mi_port_number;

    std::string proxy_name;
    std::string proxy_payload;

    int connect_to_server (
            void);

public:
    client_socket_class (
            void);

    int open_connection (
            const char* p_server_name,
            const unsigned int port_number);

    int send_and_receive (
            const unsigned char* p_buffer,
            const unsigned int size,
            unsigned char** p_answer,
            unsigned int* answer_size);

    void close_connection (
            void);

    ~client_socket_class (
            void);
};

#endif // ifndef SOCKET_CLIENT_CLASS_HPP
