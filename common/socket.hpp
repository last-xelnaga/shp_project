
#ifndef SOCKET_HPP
#define SOCKET_HPP

//#include "error_codes.hpp"

//int create_socket (
//        int* fd);

//int connect_to_server (
//        const int fd,
//        const char* p_server_name,
//        const unsigned int port_number);

//int bind_on_server (
//        const int fd,
//        const unsigned int port_number);

int set_blocking (
        const int fd,
        const bool is_blocking);

//int accept (
//        const int fd,
//        int* accept_fd);

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

//void close_socket (
//        const int fd);

#endif // SOCKET_HPP
