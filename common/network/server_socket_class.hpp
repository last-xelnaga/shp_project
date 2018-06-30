
#ifndef SOCKET_SERVER_CLASS_HPP
#define SOCKET_SERVER_CLASS_HPP

#include <atomic>
#include <list>
#include <thread>


class server_socket_class
{
private:
    class spinlock_class
    {
    private:
        std::atomic_flag flag;

    public:
        spinlock_class (
                void) : flag (ATOMIC_FLAG_INIT)
        {
        }

        void lock (
                void)
        {
            while (flag.test_and_set (std::memory_order_acquire))
            {
            }
        }

        void unlock (
                void)
        {
            flag.clear ();
        }
    };

public:
    class server_client_class;
    typedef void (*server_routine)(server_client_class*);

public:
    class server_client_class
    {
    private:
        int accept_fd;
        server_routine p_routine;
        bool is_client_running;
        std::thread m_worker_thread;

        void run (
                void);

    public:
        server_client_class (
                server_routine p_func,
                int fd);

        int recv_message (
                unsigned char** buffer,
                unsigned int* size);

        int send_message (
                const unsigned char* p_buffer,
                const unsigned int size);

        inline bool is_running (
                void)
        {
            return is_client_running;
        }

        ~server_client_class (
                void);
    };

private:
    enum socket_state_t
    {
        //ready_to_create,
        ready_to_bind,
        ready_to_accept,
        ready_to_close
    };

private:
    int socket_fd;
    unsigned int mi_port_number;
    server_routine mp_func;

    socket_state_t state;
    std::thread m_worker_thread;

    std::list <server_client_class*> m_clients;
    spinlock_class m_clients_spinlock;

private:
    int bind_on_server (
            void);

    void working_thread (
            void);

    int bind_and_listen (
            void);

    int accept_client (
            void);

    void remove_finished (
            int is_time_to_close);

public:
    server_socket_class (
            void);

    int start (
            unsigned int port_number,
            server_routine p_func);

    void terminate (
            void);

    ~server_socket_class (
            void);
};

#endif // ifndef SOCKET_SERVER_CLASS_HPP
