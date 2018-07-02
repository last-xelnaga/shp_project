
#ifndef SETTINGS_CLASS_HPP
#define SETTINGS_CLASS_HPP

#include <string>
#include <vector>


class settings_class
{
    private:
        // flag
        char is_file_processed;

        std::vector <std::pair <std::string, std::string>> m_settings;

    private:
        settings_class (
                void);

        void read_config_file (
                void);

        void init_key (
                std::string& key,
                std::string& value);

        std::string get_value (
                std::string key);

    public:
        static std::string get_value_for (
                const char* key);

    public:
        settings_class (settings_class const&) = delete;
        void operator= (settings_class const&) = delete;

};

#endif // ifndef SETTINGS_CLASS_HPP
