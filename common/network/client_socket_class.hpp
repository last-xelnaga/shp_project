
#ifndef SOCKET_CLIENT_HPP
#define SOCKET_CLIENT_HPP


class client_socket_class
{

private:
    int socket_fd;

    int connect_to_server (
            const int fd,
            const char* p_server_name,
            const unsigned int port_number);

public:
    client_socket_class (
            void);

    int connect (
            const char* p_server_name,
            const unsigned int port_number);

    int send_and_receive (
            const unsigned char* p_buffer,
            const unsigned int size,
            unsigned char** p_answer,
            unsigned int* answer_size);

    void close (
            void);

    ~client_socket_class (
            void);
};

#endif // SOCKET_CLIENT_HPP
