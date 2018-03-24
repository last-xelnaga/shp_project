
#ifndef SOCKET_SERVER_HPP
#define SOCKET_SERVER_HPP


class server_socket_class
{
public:
    class server_client_class;
    typedef void (*server_routine)(server_client_class*);

public:
    class server_client_class
    {
    private:
        int accept_fd;
        server_routine p_routine;
        
        //pthread_t thread;
        unsigned int write_timeout;
        unsigned int read_timeout;

    public:
        server_client_class (
                server_routine p_func,
                int fd);

        void run (
                void);

        int recv_message (
                unsigned char** buffer,
                unsigned int* size);

        int send_message (
                const unsigned char* p_buffer,
                const unsigned int size);

        ~server_client_class (
                void);
    };


private:
    int socket_fd;
    unsigned int listen_queue;

    long listener;
    //server_client_class* client_class;
    volatile bool terminated;

private:
    int bind_on_server (
            const unsigned int port_number);

    static void* task1 (
            void* p_huj);

    int accept_client (
            int* accept_fd);

public:
    server_socket_class (
            void);

    int bind_and_listen (
            unsigned int port_number,
            server_routine p_func);

    void terminate (
            void);

    ~server_socket_class (
            void);
};

#endif // SOCKET_SERVER_HPP
