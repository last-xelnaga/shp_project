
#ifndef SETTINGS_CLASS_HPP
#define SETTINGS_CLASS_HPP

#include <string>


class settings_class
{
    private:
        // watering settings
        unsigned int pump_start_time_in_sec;
        unsigned int pump_active_time;

        // dht settings
        unsigned int dht_sleep_time;
        unsigned int dht_last_check_time;

        // network settings
        std::string server_name;
        unsigned int server_port;

        // flag
        char is_file_processed;

    private:
        settings_class (
                void);

        void init_key (
                std::string key,
                std::string value);

        void read_config_file (
                void);

    public:
        static settings_class& get_instance (void)
        {
            static settings_class instance;
            return instance;
        }

        bool is_time_for_watering (
                void);

        unsigned int get_pump_active_time (
                void)
        {
            read_config_file ();
            return pump_active_time;
        }

        bool is_time_for_temperature (
                void);

        std::string get_server_name (
                void)
        {
            read_config_file ();
            return server_name;
        }

        unsigned int get_server_port (
                void)
        {
            read_config_file ();
            return server_port;
        }

    public:
        settings_class (settings_class const&) = delete;
        void operator= (settings_class const&) = delete;

};

#endif // SETTINGS_CLASS_HPP
