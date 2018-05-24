
#ifndef NETWORK_MANAGER_CLASS_HPP
#define NETWORK_MANAGER_CLASS_HPP

#include <list>
#include <string>


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

    void process_batch (
            void);

public:
    static network_manager_class& get_instance (
            void)
    {
        static network_manager_class instance;
        return instance;
    }

    void enqueue_message (
            std::string message);

    ~network_manager_class (
            void);

    network_manager_class (network_manager_class const&) = delete;
    void operator=(network_manager_class const&) = delete;
};

#endif // NETWORK_MANAGER_CLASS_HPP
