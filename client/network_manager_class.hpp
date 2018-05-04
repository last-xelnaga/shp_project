
#ifndef NETWORK_MANAGER_CLASS_HPP
#define NETWORK_MANAGER_CLASS_HPP

#include <string>


class network_manager_class
{
private:
    network_manager_class (
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

    network_manager_class (network_manager_class const&) = delete;
    void operator=(network_manager_class const&) = delete;
};

#endif // NETWORK_MANAGER_CLASS_HPP
