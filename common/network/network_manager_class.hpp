
#ifndef NETWORK_MANAGER_CLASS_HPP
#define NETWORK_MANAGER_CLASS_HPP

#include <list>
#include <string>
#include <vector>


class network_manager_class
{
private:
    long listener;
    std::list <std::string> m_queue;
    volatile char is_worker_active;
    unsigned long server_last_connection_time;

private:
    network_manager_class (
            void);

    static void* network_manager_worker (
            void* p_class);

public:
    static network_manager_class& get_instance (
            void)
    {
        static network_manager_class instance;
        return instance;
    }

public:
    void enqueue_message (
            std::string client,
            std::string type,
            const int status,
            std::vector <std::string> data);

    void flush (
            void);

    ~network_manager_class (
            void);

    network_manager_class (network_manager_class const&) = delete;
    void operator=(network_manager_class const&) = delete;
};

#endif // ifndef NETWORK_MANAGER_CLASS_HPP
