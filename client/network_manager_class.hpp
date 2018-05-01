
#ifndef NETWORK_MANAGER_CLASS_HPP
#define NETWORK_MANAGER_CLASS_HPP

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
            void);

    network_manager_class (network_manager_class const&) = delete;
    void operator=(network_manager_class const&) = delete;
};

#endif // NETWORK_MANAGER_CLASS_HPP
