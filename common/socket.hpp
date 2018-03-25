
#ifndef SOCKET_HPP
#define SOCKET_HPP

int set_blocking (
        const int fd,
        const bool is_blocking);

int send_data (
        const int fd,
        const unsigned char* p_buffer,
        const unsigned int total_to_send,
        const unsigned int timeout);

int recv_data (
        const int fd,
        unsigned char* p_buffer,
        const unsigned int to_receive,
        const unsigned int timeout);

#endif // SOCKET_HPP
