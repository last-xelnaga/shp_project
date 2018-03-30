
#ifndef SOCKET_CLIENT_HPP
#define SOCKET_CLIENT_HPP


class client_socket_class
{

private:
    int socket_fd;

    unsigned int connect_retry_sleep;
    unsigned int try_count_max;

    unsigned int write_timeout;
    unsigned int read_timeout;

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
