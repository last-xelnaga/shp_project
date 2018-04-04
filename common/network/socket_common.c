
#include "socket_common.h"
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>


int set_blocking (
        const int fd,
        const int is_blocking)
{
    int result = 0;
    int new_flags = -1;
    int flags = -1;
    //DEBUG_LOG_TRACE_BEGIN

    if (fd < 0)
    {
        //DEBUG_LOG_MESSAGE ("invalid socket param");
        result = -1;
    }

    // get flags
    if (result == 0)
    {
        flags = fcntl (fd, F_GETFL, 0);
        if (flags == -1)
        {
            //DEBUG_LOG_MESSAGE ("fcntl call failed: %s", strerror (errno));
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
            //DEBUG_LOG_MESSAGE ("fcntl call failed: %s", strerror (errno));
            result = -1;
        }
    }

    //DEBUG_LOG_TRACE_END (result)
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
    //DEBUG_LOG_TRACE_BEGIN

    // Initialize the set
    FD_ZERO (&writeset);
    FD_SET (fd, &writeset);

    // Initialize time out struct
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    do
    {
        memcpy (&tempset, &writeset, sizeof (writeset));

        int res = select (fd + 1, NULL, &tempset, NULL, &tv);
        if (res > 0)
        {
            int sent = send (fd, p_buffer, total_to_send_back, MSG_NOSIGNAL);
            if (sent < 0)
            {
                //DEBUG_LOG_MESSAGE ("send failed: %s", strerror (errno));
                result = -1; //RESULT_TRANSPORT_ERROR;
            }
            else if (sent == 0)
            {
                //DEBUG_LOG_MESSAGE ("send failed. channel is closed");
                result = -1; //RESULT_TRANSPORT_ERROR;
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
            //DEBUG_LOG_MESSAGE ("send failed. timeout");
            result = -1; //RESULT_TRANSPORT_ERROR;
        }
        else
        {
            // error
            //DEBUG_LOG_MESSAGE ("send failed. select error: %s", strerror (errno));
            result = -1;
        }
    } while ((result == 0) && (total_to_send_back > 0));

    //DEBUG_LOG_TRACE_END (result)
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
    //DEBUG_LOG_TRACE_BEGIN

    // Initialize the set
    FD_ZERO (&readset);
    FD_SET (fd, &readset);

    // Initialize time out struct
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    do
    {
        memcpy (&tempset, &readset, sizeof (readset));

        int res = select (fd + 1, &tempset, NULL, NULL, &tv);
        if (res > 0)
        {
            int received = recv (fd, (void*) p_recv_buffer, to_receive - total_received, 0);
            if (received < 0)
            {
                // error
                //DEBUG_LOG_MESSAGE ("recv failed: %s", strerror (errno));
                result = -1; //RESULT_TRANSPORT_ERROR;
            }
            else if (received == 0)
            {
                //DEBUG_LOG_MESSAGE ("recv failed. channel is closed");
                result = -1; //RESULT_TRANSPORT_ERROR;
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
            //DEBUG_LOG_MESSAGE ("recv failed. timeout");
            result = -1; //RESULT_TRANSPORT_ERROR;
        }
        else
        {
            // error
            //DEBUG_LOG_MESSAGE ("recv failed. select error: %s", strerror (errno));
            result = -1;
        }
    } while ((result == 0) && (total_received < to_receive));

    //DEBUG_LOG_TRACE_END (result)
    return result;
}