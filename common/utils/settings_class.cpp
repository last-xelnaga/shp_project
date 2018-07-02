
#include "file_utils.h"
#include "log.h"
#include "settings_class.hpp"

#include <fstream>
#include <pthread.h>
#include <sstream>

#define SETTINGS_FILE_NAME      "settings.cfg"  // name of the file with settings (optional)


pthread_mutex_t settings_mutex = PTHREAD_MUTEX_INITIALIZER;

// private methods
settings_class::settings_class (
        void)
{
    is_file_processed = 0;
}

void settings_class::read_config_file (
        void)
{
    // quick check for the already finished initialization
    if (is_file_processed)
        return;

    // otherwise we have to try to read the settings file
    // enter the critical section, lock the mutex
    pthread_mutex_lock (&settings_mutex);

    do
    {
        // check it again
        if (is_file_processed)
            break;

        LOG_INFO ("read_config_file");

        if (does_file_exist (SETTINGS_FILE_NAME) != 1)
        {
            LOG_ERROR ("%s does not exist", SETTINGS_FILE_NAME);
            break;
        }

        std::stringstream buffer;
        std::ifstream file (SETTINGS_FILE_NAME);
        if (file)
        {
            buffer << file.rdbuf ();
            file.close ();
        }
        else
        {
            LOG_ERROR ("failed to open %s", SETTINGS_FILE_NAME);
            break;
        }

        std::string line;
        while (std::getline (buffer, line))
        {
            std::istringstream is_line (line);

            std::string key, value;
            if (std::getline (is_line, key, '='))
            {
                if (std::getline (is_line, value))
                    init_key (key, value);
            }
        }
    }
    while (0);

    is_file_processed = 1;

    // we have finished our work with file so unlock the mutex
    pthread_mutex_unlock (&settings_mutex);
}

void settings_class::init_key (
        std::string& key,
        std::string& value)
{
    for (unsigned int i = 0; i < m_settings.size (); ++ i)
        if (m_settings [i].first == key)
        {
            LOG_INFO ("duplicate key \"%s\", skip", key.c_str ());
            return;
        }

    m_settings.push_back (std::make_pair (key, value));
    LOG_INFO ("add key \"%s\" = \"%s\"", key.c_str (), value. c_str ());
}

std::string settings_class::get_value (
        std::string key)
{
    std::string result = "0";

    for (unsigned int i = 0; i < m_settings.size (); ++ i)
        if (m_settings [i].first == key)
        {
            result = m_settings [i].second;
            break;
        }

    return result;
}

std::string settings_class::get_value_for (
        const char* key)
{
    static settings_class instance;
    instance.read_config_file ();
    return instance.get_value (std::string (key));
}
