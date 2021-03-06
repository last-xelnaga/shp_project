
#include "log.h"
#include "socket_common.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>


int set_blocking (
        const int fd,
        const int is_blocking)
{
    int result = 0;
    int new_flags = -1;
    int flags = -1;

    if (fd < 0)
    {
        LOG_ERROR ("invalid socket param");
        result = -1;
    }

    // get flags
    if (result == 0)
    {
        flags = fcntl (fd, F_GETFL, 0);
        if (flags == -1)
        {
            LOG_ERROR ("fcntl call failed: %s", strerror (errno));
            result = -1;
        }
    }

    // prepare new flags
    if (result == 0)
    {
        new_flags = flags | O_NONBLOCK;
        if (is_blocking == 0)
        {
            new_flags = flags & ~O_NONBLOCK;
        }
    }

    // set flags
    if (result == 0)
    {
        flags = fcntl (fd, F_SETFL, new_flags);
        if (flags == -1)
        {
            LOG_ERROR ("fcntl call failed: %s", strerror (errno));
            result = -1;
        }
    }

    return result;
}

int send_data (
        const int fd,
        const unsigned char* p_buffer,
        const unsigned int total_to_send,
        const unsigned int timeout)
{
    int result = 0;
    struct timeval tv;
    fd_set writeset, tempset;
    unsigned int total_to_send_back = total_to_send;

    // Initialize the set
    FD_ZERO (&writeset);
    FD_SET (fd, &writeset);

    do
    {
        // Initialize time out struct
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        memcpy (&tempset, &writeset, sizeof (writeset));

        int res = select (fd + 1, NULL, &tempset, NULL, &tv);
        if (res > 0)
        {
            int sent = send (fd, p_buffer, total_to_send_back, MSG_NOSIGNAL);
            if (sent < 0)
            {
                LOG_ERROR ("send failed: %s", strerror (errno));
                result = -1;
            }
            else if (sent == 0)
            {
                LOG_ERROR ("send failed. channel is closed");
                result = -1;
            }
            else
            {
                total_to_send_back -= sent;
                p_buffer += sent;
            }
        }
        else if (res == 0)
        {
            // time out
            LOG_ERROR ("send failed. timeout");
            result = -1;
        }
        else
        {
            // error
            LOG_ERROR ("send failed. select error: %s", strerror (errno));
            result = -1;
        }
    } while ((result == 0) && (total_to_send_back > 0));

    return result;
}

int recv_data (
        const int fd,
        unsigned char* p_buffer,
        const unsigned int to_receive,
        const unsigned int timeout)
{
    int result = 0;
    struct timeval tv;
    fd_set readset, tempset;
    unsigned int total_received = 0;
    unsigned char* p_recv_buffer = (unsigned char*) p_buffer;

    // Initialize the set
    FD_ZERO (&readset);
    FD_SET (fd, &readset);

    do
    {
        // Initialize time out struct
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        memcpy (&tempset, &readset, sizeof (readset));

        int res = select (fd + 1, &tempset, NULL, NULL, &tv);
        if (res > 0)
        {
            int received = recv (fd, (void*) p_recv_buffer, to_receive - total_received, 0);
            if (received < 0)
            {
                // error
                LOG_ERROR ("recv failed: %s", strerror (errno));
                result = -1;
            }
            else if (received == 0)
            {
                LOG_ERROR ("recv failed. channel is closed");
                result = -1;
            }
            else
            {
                // ok receive
                total_received += received;
                p_recv_buffer += received;
            }
        }
        else if (res == 0)
        {
            // time out
            LOG_ERROR ("recv failed. timeout");
            result = -1;
        }
        else
        {
            // error
            LOG_ERROR ("recv failed. select error: %s", strerror (errno));
            result = -1;
        }
    } while ((result == 0) && (total_received < to_receive));

    return result;
}
