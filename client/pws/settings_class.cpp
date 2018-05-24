
#include "file.h"
#include "log.h"
#include "settings_class.hpp"

#include <fstream>
#include <pthread.h>
#include <sstream>

// default settings
#define PUMP_START_TIME         1500    // start every day at 15:00
#define PUMP_ACTIVE_TIME        90      // how many seconds pump has to work
#define DHT_SLEEP_TIME          10      // check temp every 10 sec
#define SERVER_NAME             "localhost"     // 127.0.0.1
#define SERVER_PORT             5000
#define SERVER_BATCH_CYCLE      60      // try to batch upload every min

#define SETTINGS_FILE_NAME      "settings.cfg"  // name of the file with settings (optional)


pthread_mutex_t settings_mutex = PTHREAD_MUTEX_INITIALIZER;

// private methods
settings_class::settings_class (
        void)
{
    pump_start_time_in_sec = PUMP_START_TIME;
    pump_active_time = PUMP_ACTIVE_TIME;
    dht_sleep_time = DHT_SLEEP_TIME;
    dht_last_check_time = 0;
    server_name = SERVER_NAME;
    server_port = SERVER_PORT;
    server_batch_cycle = SERVER_BATCH_CYCLE;

    is_file_processed = 0;
}

void settings_class::init_key (
        std::string key,
        std::string value)
{
    if (key == "pump_start_time")
    {
        int pump_time = std::stoi (value);
        pump_start_time_in_sec = pump_time / 100 * 3600 + pump_time % 100 * 60;
        DEBUG_LOG_INFO ("settings: \"pump_start_time_in_sec\" = %d", pump_start_time_in_sec);
    }
    else if (key == "pump_active_time")
    {
        pump_active_time = std::stoi (value);
        DEBUG_LOG_INFO ("settings: \"pump_active_time\" = %d", pump_active_time);
    }
    else if (key == "dht_sleep_time")
    {
        dht_sleep_time = std::stoi (value);
        DEBUG_LOG_INFO ("settings: \"dht_sleep_time\" = %d", dht_sleep_time);
    }
    else if (key == "server_name")
    {
        server_name = value;
        DEBUG_LOG_INFO ("settings: \"server_name\" = %s", server_name.c_str ());
    }
    else if (key == "server_port")
    {
        server_port = std::stoi (value);
        DEBUG_LOG_INFO ("settings: \"server_port\" = %d", server_port);
    }
    else if (key == "server_batch_cycle")
    {
        server_batch_cycle = std::stoi (value);
        DEBUG_LOG_INFO ("settings: \"server_batch_cycle\" = %d", server_batch_cycle);
    }
    else
    {
        DEBUG_LOG_ERROR ("unknown key \"%s\"", key.c_str ());
    }
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

    // check it again
    if (is_file_processed)
        return;

    DEBUG_LOG_INFO ("read_config_file");

    std::stringstream buffer;

    int status = does_file_exist (SETTINGS_FILE_NAME);
    if (status == 1)
    {
        std::ifstream file (SETTINGS_FILE_NAME);
        if (file)
        {
            buffer << file.rdbuf ();
            file.close ();
        }
        else
        {
            status = 0;
            DEBUG_LOG_ERROR ("failed to open %s", SETTINGS_FILE_NAME);
        }
    }

    if (status == 0)
    {
        DEBUG_LOG_ERROR ("settings will use default values");
    }

    if (status == 1)
    {
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

    is_file_processed = 1;

    // we have finished our work with file so unlock the mutex
    pthread_mutex_unlock (&settings_mutex);
}


// public methods
bool settings_class::is_time_for_watering (
        void)
{
    read_config_file ();

    time_t now = time (NULL);
    struct tm tm = *localtime (&now);
    unsigned int curr_time_in_sec = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
    unsigned int pump_stop_time_in_sec = pump_start_time_in_sec + pump_active_time - 1;

    //DEBUG_LOG_ERROR ("curr_time_in_sec %d, pump_start_time_in_sec %d", curr_time_in_sec, pump_start_time_in_sec);

    if (curr_time_in_sec >= pump_start_time_in_sec && curr_time_in_sec <= pump_stop_time_in_sec)
        return true;

    return false;
}

bool settings_class::is_time_for_temperature (
        void)
{
    read_config_file ();

    time_t now = time (NULL);

    //DEBUG_LOG_ERROR ("dht_last_check_time %d, now %ld", dht_last_check_time, now);

    if (now - dht_last_check_time >= dht_sleep_time)
    {
        dht_last_check_time = now;
        return true;
    }

    return false;
}
