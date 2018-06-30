
#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

int set_blocking (
        const int fd,
        const int is_blocking);

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

#ifdef __cplusplus
}
#endif

#endif // ifndef SOCKET_COMMON_H
